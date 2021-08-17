#include "sctcpsocketthread.h"

#include <QTcpSocket>
#include <QDateTime>
#include <QDebug>
#include <QTimer>

#include "SCHeadData.h"

SCTcpSocketThread::SCTcpSocketThread(qintptr socketDescriptor, QObject *parent) : QThread(parent), m_socketDescriptor(socketDescriptor)
{
    moveToThread(this);
}

SCTcpSocketThread::~SCTcpSocketThread()
{
    if(m_pHeartbeatTimer)
    {
        m_pHeartbeatTimer->deleteLater();
    }
    if(m_pTcpSocket)
    {
        m_pTcpSocket->deleteLater();
    }
}

Qt::HANDLE SCTcpSocketThread::getThreadId()
{
    return m_threadId;
}

void SCTcpSocketThread::run()
{
    m_pHeartbeatTimer = new QTimer;
    m_pHeartbeatTimer->setInterval(31000);
    connect(m_pHeartbeatTimer, &QTimer::timeout, this, &SCTcpSocketThread::onHeartbeatTimeout);

    if(!m_pTcpSocket)
    {
        m_pTcpSocket = new QTcpSocket;
        m_pTcpSocket->setSocketDescriptor(m_socketDescriptor);
    }
    connect(m_pTcpSocket, &QTcpSocket::readyRead, this, &SCTcpSocketThread::onReadReadyed);
    connect(m_pTcpSocket, &QTcpSocket::disconnected, this, &SCTcpSocketThread::onTcpSocketDisconnected);
    m_threadId = currentThreadId();
    qDebug() << currentThreadId();
    exec();
}

void SCTcpSocketThread::readFailed()
{
    qDebug() << "read failed";
    m_lastData.clear();
    m_readStatus = Failed;
    disconnectTcpSocket();
}

void SCTcpSocketThread::disconnectTcpSocket()
{
    if(m_pTcpSocket->isOpen())
    {
        m_pTcpSocket->close();
        m_pTcpSocket->waitForDisconnected();
    }
}

void SCTcpSocketThread::onReadReadyed()
{
    m_readStatus = Reading;
    QByteArray data = m_pTcpSocket->readAll();
    int size = data.size();
    // 心跳检测
    if((uint8_t)data.at(0) == 0x41)
    {
        static int i = 0;
        i++;
        qDebug() << QString("heartbeat%1").arg(i);
        m_pHeartbeatTimer->stop();
        m_pHeartbeatTimer->start();
        return;
    }

    uint32_t length;  // 数据区长度
    uint16_t type;    // 报文类型
    uint16_t number;  // 序列号
    while(size > 0)
    {
        char a0 = data.at(0);
        if(uint8_t(a0) == 0x5A)
        {
            if(size >= 16)
            {
                SeerHeader* header = new SeerHeader;
                memcpy(header, data.data(), 16);

                qToBigEndian(header->m_length, (uint8_t*)&(length));
                qToBigEndian(header->m_type, (uint8_t*)&(type));
                qToBigEndian(header->m_number, (uint8_t*)&(number));
                delete header;

                int remaining_size = size - 16;
                if((int)length > remaining_size)
                {
                    m_lastData = data;
                    break;
                }
                else
                {
                    QByteArray head = data.left(16);
                    QByteArray json_data = data.mid(16, length);

                    QString info = QString("%1-----------[Response]----------\n"
                                           "Type: %2 (0x%3) \n"
                                           "Number: %4 (0x%5)\n"
                                           "Head hex: %6\n"
                                           "Data[size:%7 (0x%8)]: %9\n"
                                           "Data hex: %10\n")
                            .arg(QDateTime::currentDateTime().toString("[yyyyMMdd|hh:mm:ss:zzz]:"))
                            .arg(type)
                            .arg(QString::number(type, 16))
                            .arg(number)
                            .arg(QString::number(number, 16))
                            .arg(QString::fromUtf8(head.toHex()))
                            .arg(json_data.size())
                            .arg(QString::number(json_data.size(), 16))
                            .arg(QString::fromUtf8(json_data))
                            .arg(QString::fromUtf8(json_data.toHex()));
                    emit signalReadyRead(info);
                    m_lastData.clear();
                    m_readStatus = Completed;
                    break;
                }
            }
            else
            {
                readFailed();
                break;
            }
        }
        else
        {
            readFailed();
            break;
        }
    }

    // 开始写
    if(m_readStatus == Completed)
    {
        uint8_t* headBuf = Q_NULLPTR;
        int headSize;

        SeerData* pSeerData = nullptr;
        std::string json_str = "";
        switch (type)
        {
        case 1000: /*查询机器人信息*/
            json_str = "{\"id\":\"S001\",\"version\":\"v1.1.0\",\"model\":\"S1\",\"dsp_version\":\"v1.2.2\",\"map_version\":\"v1.0.0\",\"model_version\":\"v1.1.0\",\"netprotocol_version\":\"v1.2.0\"}";
            break;
        case 1002: /*查询机器人运行信息*/
            json_str = "{\"controller_humi\":\"50.0\",\"controller_temp\":\"30.0\",\"controller_voltage\":\"24.0\",\"odo\":\"146.34\",\"time\":\"794297\",\"total_time\":\"116606929\"}";
            break;
        case 1004: /*查询机器人位置*/
            json_str = "{\"angle\":\"-0.0064\",\"confidence\":\"0.637\",\"x\":\"3.5069\",\"y\":\"0.0687\",\"current_station\":\"LM1\",\"last_station\":\"LM2\"}";
            break;
        case 1005: /*查询机器人速度*/
            json_str = "{\"r_vx\":\"0.0\",\"r_vy\":\"0.0\",\"r_w\":\"0.0\",\"vx\":\"0.0\",\"vy\":\"0.0\",\"w\":\"0.0\"}";
            break;
        case 1006: /*查询机器人的被阻挡状态*/
            break;
        case 1007: /*查询机器人电池状态*/
            json_str = "{\"battery_level\":\"0.87\",\"battery_temp\":\"35.0\",\"charging\":\"false\",\"current\":\"2.0\",\"voltage\":\"24.5\"}";
            break;
        case 1040: /*查询电机状态*/
            json_str = "{\"motor_info\":[{\"can_id\":\"2\",\"can_router\":\"2\",\"emc\":\"false\",\"err\":\"false\",\"error_code\":\"0\",\"motor_name\":\"motor1\",\"position\":\"0\",\"speed\":\"0\",\"stop\":\"false\"}],\"ret_code\":\"0\"}";
            break;
        case 1100:
            json_str = "{\"x\": 10.0 \"y\" : 3.0, \"angle\": 0}";
            break;

        default:
            break;
        }

        headSize = sizeof(SeerHeader) + json_str.length();
        headBuf = new uint8_t[headSize];
        pSeerData = (SeerData*)headBuf;
        size = pSeerData->setData(type + (uint16_t)10000,
                                 (uint8_t*)json_str.data(),
                                 json_str.length(),
                                 number);

        m_pTcpSocket->write((char*)pSeerData, size);
        m_readStatus = UnStart;
    }
}

void SCTcpSocketThread::onTcpSocketDisconnected()
{
    qDebug() << "tcpSocket Disconnected";
    emit signalTcpSocketDisconnected();
}

void SCTcpSocketThread::onHeartbeatTimeout()
{
    qDebug() << "heartbeat timeout, disconnect client";
    disconnectTcpSocket();
}
