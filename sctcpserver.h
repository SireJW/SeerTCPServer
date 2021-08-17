#ifndef SCTCPSERVER_H
#define SCTCPSERVER_H

#include <QObject>
#include <QVector>

class SCTcpSocketThread;
class QTcpServer;
class SCTcpServer : public QObject
{
    Q_OBJECT

public:
    SCTcpServer(QObject *parent = nullptr);
    ~SCTcpServer();

signals:
    void signalConnectionSucceed(QString strHostAddress, int nPort);
    void signalDataReaded(QString data);
    void signalSocketNumChanged(int);

private slots:
    void onNewConnectionAdded();
    void onTcpSocketDisconnected();
    void onThreadFinished();

private:
    QTcpServer* m_pTcpServer{Q_NULLPTR};        // 服务器
    QVector<SCTcpSocketThread*> m_vecThread;    // 客户端socket线程
};

#endif // SCTCPSERVER_H
