#include "sctcpserverwidget.h"
#include "ui_sctcpserverwidget.h"

#include "sctcpserverthread.h"

SCTcpServerWidget::SCTcpServerWidget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::SCTcpServerWidget)
{
    ui->setupUi(this);
    QVector<int> vecPort;
    vecPort << 19204 << 19205 << 19206 << 19207 << 19208 << 19209 << 19210 << 19200;
    foreach (auto port, vecPort)
    {
        m_vecServerThread.push_back(new SCTcpServerThread(port));
    }
    foreach (auto thread, m_vecServerThread)
    {
        connect(thread, &SCTcpServerThread::signalConnectionSucceed, this, &SCTcpServerWidget::onConnectionSucceed);
        connect(thread, &SCTcpServerThread::signalDataReaded, this, &SCTcpServerWidget::onDataReaded);
        thread->start();
    }
}

SCTcpServerWidget::~SCTcpServerWidget()
{
    delete ui;

    foreach(auto thread, m_vecServerThread)
    {
        thread->quit();
        thread->wait();
        thread->deleteLater();
    }
}

void SCTcpServerWidget::onConnectionSucceed(QString strHostAddress, int nPort)
{
    ui->lineEdit->setText(QString("connection succeed"));
    ui->lineEdit_2->setText(strHostAddress);
    ui->lineEdit_3->setText(QString::number(nPort));
}

void SCTcpServerWidget::onDataReaded(QString data)
{
    ui->textEdit->setText(data);
}

void SCTcpServerWidget::onSocketNumChanged(int nNum)
{
    Q_UNUSED(nNum)
    //ui->lineEdit_4->setText(QString::number(nNum));
}
