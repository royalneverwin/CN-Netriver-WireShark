/*
* THIS FILE IS FOR IP TEST
*/
// system support
#include "sysInclude.h"

extern void ip_DiscardPkt(char* pBuffer, int type);

extern void ip_SendtoLower(char* pBuffer, int length);

extern void ip_SendtoUp(char* pBuffer, int length);

extern unsigned int getIpv4Address();

// implemented by students

const int discardType[5] = {
    STUD_IP_TEST_CHECKSUM_ERROR,
    STUD_IP_TEST_TTL_ERROR,
    STUD_IP_TEST_VERSION_ERROR,
    STUD_IP_TEST_HEADLEN_ERROR,
    STUD_IP_TEST_DESTINATION_ERROR
};

// 检查头校验和
bool checkHeadSum(unsigned int* bufferHead, int length) {
    unsigned int sum = 0;
    for (int i = 0; i < length; i++) {
        sum += (bufferHead[i] >> 16) & 0xffff; // calculate the sum
        sum = (sum >> 16) + (sum & 0xffff); // wrap around when overflow
        sum += bufferHead[i] & 0xffff; // calculate the sum
        sum = (sum >> 16) + (sum & 0xffff); // wrap around when overflow
    }

    if ((unsigned short)~sum == 0) // 取反码为0则保留
        return true;

    return false;
}

// 检查头部字段 0表示没问题 1表示校验和错 2表示TTL出错 3表示IP版本号错 4表示头部长度错 5表示目的地址错
int checkSegment(unsigned int* pBuffer, unsigned int& headLen) {
    unsigned int bufferHead[5];
    for (int i = 0; i < 10; i++) {
        bufferHead[i] = ntohl(pBuffer[i]);
    }

    headLen = (bufferHead[0] >> 24) & 0xf;

    // cout << "headLen: " << headLen << endl;
    if (!checkHeadSum(bufferHead, 5)) { // 校验和错
        return 1;
    }

    if (char((bufferHead[2] >> 24) & 0xff) <= 0) { // TTL错
        return 2;
    }

    if ((bufferHead[0] >> 28) & 0xf != 4) { // IP版本号错
        return 3;
    }

    if (((bufferHead[0] >> 24) & 0xf) * 4 < 20) { // 头部长度错
        return 4;
    }

    if (bufferHead[4] != getIpv4Address()) { // 目的地址错
        return 5;
    }

    return 0;
}

int stud_ip_recv(char* pBuffer, unsigned short length)
{
    unsigned int headLen;
    // cout << "length: " << length << endl;
    int type = checkSegment((unsigned int*)pBuffer, headLen);
    // cout << "Type: " << type << endl;
    if (type != 0) { // 丢弃
        ip_DiscardPkt(pBuffer, discardType[type - 1]);
        return 1;
    }
    else {
        ip_SendtoUp((pBuffer + 4 * headLen), (int)length - 4 * headLen);
        return 0;
    }
}

int stud_ip_Upsend(char* pBuffer, unsigned short len, unsigned int srcAddr,
    unsigned int dstAddr, byte protocol, byte ttl)
{
    unsigned short totalLen = len + 20;
    unsigned int* sendBuffer = (unsigned int*)malloc(sizeof(char) * totalLen);
    sendBuffer[0] = (4 << 28) | (5 << 24) | totalLen;
    sendBuffer[1] = 0;
    sendBuffer[2] = (ttl << 24) | (protocol << 16);
    sendBuffer[3] = srcAddr;
    sendBuffer[4] = dstAddr;

    // 计算检验和
    unsigned int sum = 0;
    for (int i = 0; i < 5; i++) {
        sum += (sendBuffer[i] >> 16) & 0xffff; // calculate the sum
        sum = (sum >> 16) + (sum & 0xffff); // wrap around when overflow
        sum += sendBuffer[i] & 0xffff; // calculate the sum
        sum = (sum >> 16) + (sum & 0xffff); // wrap around when overflow
    }

    sum = ~sum;
    sendBuffer[2] += (unsigned short)sum;

    // 对首部要全部转成网络序
    for (int i = 0; i < 5; i++) {
        sendBuffer[i] = htonl(sendBuffer[i]);
    }

    memcpy(&(sendBuffer[5]), pBuffer, len);

    ip_SendtoLower((char*)sendBuffer, totalLen);

    return 0;
}
