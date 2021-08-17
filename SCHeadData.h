#ifndef SCHEADDATA_H
#define SCHEADDATA_H

#include <stdint.h>
#include <QtEndian>

class SeerHeader{
public:
    SeerHeader(){
        m_header = 0x5A;     // 头
        m_version = 0x01;    // 版本
        m_number = 0;        // 数量
        m_length = 0;        // 长度
        m_type = 0;          // 类型
        memset(m_reserved,0, 6);
    }

    // unsigned char
    // unsigned short
    // unsigned int
    uint8_t m_header;
    uint8_t m_version;
    uint16_t m_number;   // 序号
    uint32_t m_length;
    uint16_t m_type;     // 报文类型： type
    uint8_t m_reserved[6];
};


class SeerData{
public:
    SeerData(){}
    ~SeerData(){}
    int size(){
        return sizeof(SeerHeader) + m_header.m_length;
    }

    int setData(uint16_t type, uint8_t* data = NULL, int size = 0, uint16_t number = 0x00){
        m_header.m_header = 0x5A;
        m_header.m_version = 0x01;
        qToBigEndian(type, (uint8_t*)&(m_header.m_type));
        qToBigEndian(number, (uint8_t*)&(m_header.m_number));
        memset(m_header.m_reserved,0, 6);
        if (data != NULL) {
            memcpy(m_data, data, size);
        }
        qToBigEndian(size, (uint8_t*)&(m_header.m_length));
        return 16 + size;
    }
private:
    SeerHeader m_header; // 头
    uint8_t m_data[1];   // 数据区
};

#endif // SCHEADDATA_H
