#include "sctcpserver.h"

#include <QTcpServer>
#include <QTcpSocket>
#include <QDebug>

#include "SCHeadData.h"
#include "sctcpsocketthread.h"

SCTcpServer::SCTcpServer(QObject *parent) : QObject(parent)
{
    m_pTcpServer = new QTcpServer(this);
    m_pTcpServer->listen(QHostAddress::AnyIPv4, 19204);
    connect(m_pTcpServer, &QTcpServer::newConnection, this, &SCTcpServer::onNewConnectionAdded);
}

SCTcpServer::~SCTcpServer()
{
    foreach(auto thread, m_vecThread)
    {
        m_vecThread.removeOne(thread);
        thread->quit();
        thread->wait();
        thread->deleteLater();
    }
}

void SCTcpServer::onNewConnectionAdded()
{
    if(!m_pTcpServer->hasPendingConnections())
        return;

    QTcpSocket* pTcpSocket = m_pTcpServer->nextPendingConnection();
    if(!pTcpSocket->isValid())
        return;
    emit signalConnectionSucceed(pTcpSocket->peerAddress().toString(), pTcpSocket->peerPort());

    SCTcpSocketThread* pThread = new SCTcpSocketThread(pTcpSocket->socketDescriptor());
    connect(pThread, &SCTcpSocketThread::finished, this, [=](){  });
    connect(pThread, &SCTcpSocketThread::signalReadyRead, this, &SCTcpServer::signalDataReaded);
    connect(pThread, &SCTcpSocketThread::signalTcpSocketDisconnected, this, &SCTcpServer::onTcpSocketDisconnected);
    m_vecThread.push_back(pThread);
    pThread->start();
    qDebug() << "连接一个客户端，threadId:";
    emit signalSocketNumChanged(m_vecThread.count());
}

void SCTcpServer::onTcpSocketDisconnected()
{
    auto pThread = qobject_cast<SCTcpSocketThread*>(sender());
    if(!pThread)
        return;

    foreach(auto thread, m_vecThread)
    {
        if(thread == pThread)
        {
            qDebug() << "断开一个客户端，threadId:" << thread->getThreadId();
            m_vecThread.removeAll(thread);
            thread->quit();
            thread->wait();
            thread->deleteLater();
            break;
        }
    }
    emit signalSocketNumChanged(m_vecThread.count());
}

void SCTcpServer::onThreadFinished()
{
    auto pThread = qobject_cast<SCTcpSocketThread*>(sender());
    if(!pThread)
        return;

    foreach(auto thread, m_vecThread)
    {
        if(thread == pThread)
        {
            m_vecThread.removeOne(thread);
            thread->quit();
            thread->wait();
            thread->deleteLater();
            break;
        }
    }
    emit signalSocketNumChanged(m_vecThread.count());
}
