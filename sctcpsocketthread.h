#ifndef SCTCPSOCKETTHREAD_H
#define SCTCPSOCKETTHREAD_H

#include <QThread>

class QTcpSocket;
class QTimer;
class SCTcpSocketThread : public QThread
{
    Q_OBJECT
public:
    explicit SCTcpSocketThread(qintptr socketDescriptor, QObject *parent = 0);
    virtual ~SCTcpSocketThread();

    Qt::HANDLE getThreadId();
    void disconnectTcpSocket();

signals:
    void signalReadyRead(QString data);
    void signalTcpSocketDisconnected();

protected:
    void run();

private:
    void readFailed();

private slots:
    void onReadReadyed();
    void onTcpSocketDisconnected();
    void onHeartbeatTimeout();

private:
    enum ReadStatus
    {
        UnStart,
        Reading,
        Completed,
        Failed
    };
    QTcpSocket* m_pTcpSocket{Q_NULLPTR};  // socket
    qintptr m_socketDescriptor;           // socket描述符
    Qt::HANDLE m_threadId{(void*)0x00};   // 线程id

    QByteArray m_lastData;                // 数据
    ReadStatus m_readStatus{UnStart};     // 读状态
    QTimer* m_pHeartbeatTimer{Q_NULLPTR}; // 心跳检测计时器

};

#endif // SCTCPSOCKETTHREAD_H
