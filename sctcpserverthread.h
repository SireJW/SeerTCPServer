#ifndef SCTCPSERVERTHREAD_H
#define SCTCPSERVERTHREAD_H

#include <QThread>
#include <QVector>

class QTcpServer;
class SCTcpSocketThread;
class SCTcpServerThread : public QThread
{
    Q_OBJECT
public:
    explicit SCTcpServerThread(int port, QObject *parent = nullptr);
    ~SCTcpServerThread();

signals:
    void signalConnectionSucceed(QString strHostAddress, int nPort);
    void signalDataReaded(QString data);

protected:
    void run();

private slots:
    void onNewConnectionAdded();
    void onTcpSocketDisconnected();

private:
    QTcpServer* m_pTcpServer{Q_NULLPTR};        // 服务器
    QVector<SCTcpSocketThread*> m_vecThread;    // 客户端socket线程
    int m_nPort;                                // 监听端口号
};

#endif // SCTCPSERVERTHREAD_H
