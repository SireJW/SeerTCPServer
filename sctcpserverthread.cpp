#include "sctcpserverthread.h"

#include <QTcpServer>
#include <QTcpSocket>

#include <sctcpsocketthread.h>

SCTcpServerThread::SCTcpServerThread(int port, QObject *parent) : QThread(parent), m_nPort(port)
{
    moveToThread(this);
}

SCTcpServerThread::~SCTcpServerThread()
{
    foreach(auto thread, m_vecThread)
    {
        m_vecThread.removeOne(thread);
        thread->disconnectTcpSocket();
        thread->quit();
        thread->wait();
        thread->deleteLater();
    }

    if(m_pTcpServer)
    {
        m_pTcpServer->close();
        m_pTcpServer->deleteLater();
    }
}

void SCTcpServerThread::run()
{
    m_pTcpServer = new QTcpServer;
    m_pTcpServer->listen(QHostAddress::AnyIPv4, m_nPort);
    connect(m_pTcpServer, &QTcpServer::newConnection, this, &SCTcpServerThread::onNewConnectionAdded);
    exec();
 }


void SCTcpServerThread::onNewConnectionAdded()
{
    if(!m_pTcpServer->hasPendingConnections())
        return;

    QTcpSocket* pTcpSocket = m_pTcpServer->nextPendingConnection();
    if(!pTcpSocket->isValid())
        return;
    emit signalConnectionSucceed(pTcpSocket->peerAddress().toString(), pTcpSocket->peerPort());

    SCTcpSocketThread* pThread = new SCTcpSocketThread(pTcpSocket->socketDescriptor());
    connect(pThread, &SCTcpSocketThread::signalReadyRead, this, &SCTcpServerThread::signalDataReaded);
    connect(pThread, &SCTcpSocketThread::signalTcpSocketDisconnected, this, &SCTcpServerThread::onTcpSocketDisconnected);
    m_vecThread.push_back(pThread);
    pThread->start();
    qDebug() << m_nPort << "连接一个客户端，threadId:";
}

void SCTcpServerThread::onTcpSocketDisconnected()
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
}
