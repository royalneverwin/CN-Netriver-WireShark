/*
* THIS FILE IS FOR TCP TEST
*/

/*
struct sockaddr_in {
		short   sin_family;
		u_short sin_port;
		struct  in_addr sin_addr;
		char    sin_zero[8];
};
*/

#include "sysInclude.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <vector>
#include <algorithm>
using namespace std;

extern void tcp_DiscardPkt(char* pBuffer, int type);

extern void tcp_sendReport(int type);

extern void tcp_sendIpPkt(unsigned char* pData, UINT16 len, unsigned int  srcAddr, unsigned int dstAddr, UINT8	ttl);

extern int waitIpPacket(char* pBuffer, int timeout);

extern unsigned int getIpv4Address();

extern unsigned int getServerIpv4Address();




int gSrcPort = 2007;
int gDstPort = 2006;
int gSeqNum = 1;
int gAckNum = 1;

typedef uint8_t State;
enum TCPStatus { CLOSED, SYN_SENT, ESTABLISHED, FIN_WAIT1, FIN_WAIT2, TIME_WAIT };

typedef struct TCPHead {
	uint16_t srcPort;
	uint16_t dstPort;
	uint32_t seqNum;
	uint32_t ackNum;
	uint8_t headLen;
	uint8_t type;
	uint16_t windowSize;
	uint16_t checkSum;
	uint16_t urgentPointer;
	char data[512];
	void ntoh() {
		srcPort = ntohs(srcPort);
		dstPort = ntohs(dstPort);
		seqNum = ntohl(seqNum);
		ackNum = ntohl(ackNum);
		windowSize = ntohs(windowSize);
		checkSum = ntohs(checkSum);
		urgentPointer = ntohs(urgentPointer);
		int head_len_left = ((headLen >> 4) & 0xf) - 5;
		for (int i = 0; i < head_len_left; i++) {
			((uint32_t*)data)[i] = ntohl(((uint32_t*)data)[i]);
		}
	}
	void hton() {
		int head_len_left = ((headLen >> 4) & 0xf) - 5;
		srcPort = htons(srcPort);
		dstPort = htons(dstPort);
		seqNum = htonl(seqNum);
		ackNum = htonl(ackNum);
		windowSize = htons(windowSize);
		checkSum = htons(checkSum);
		urgentPointer = htons(urgentPointer);
		for (int i = 0; i < head_len_left; i++) {
			((uint32_t*)data)[i] = htonl(((uint32_t*)data)[i]);
		}
	}
	void display() {
		printf("++++++++++++ tcp_head display -----------\n");
		printf("srcPort dstPort (%d, %d)\n", srcPort, dstPort);
		printf("seqNum ackNum (%d, %d)\n", seqNum, ackNum);
		printf("headLen flag %d, %d\n", ((headLen >> 4) & 0xf), type);
		printf("windowSize, checkSum, urgentPointer (%d, %d, %d)\n", windowSize, checkSum, urgentPointer);
		printf("------------ endof -----------\n");
	}
};

typedef struct MyTCB {
	uint32_t srcAddr;
	uint32_t dstAddr;
	uint16_t srcPort;
	uint16_t dstPort;
	uint32_t seq;
	uint32_t ack;
	State status;
	unsigned int ackExpect; // ???????????????ack
	int sockfd; // ??????????????????
	char* data; // TCB???????????????
};

int TCBNum = 0;
vector<MyTCB*> TCBTable;
MyTCB* curTCB;

// ???????????????????????????????????????????????????
// ?????????TCP??????????????????????????????????????????????????????TCP????????????????????????????????????????????????6
unsigned int GetCheckSum(TCPHead* head, unsigned short len, unsigned int srcAddr, unsigned int dstAddr) {
	unsigned int sum = 0;
	// ?????????
	sum += (srcAddr >> 16) & 0xffff;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += srcAddr & 0xffff;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (dstAddr >> 16) & 0xffff;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += dstAddr & 0xffff;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += IPPROTO_TCP;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += len;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += head->srcPort;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += head->dstPort;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += ((head->seqNum) >> 16) & 0xffff;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (head->seqNum) & 0xffff;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += ((head->ackNum) >> 16) & 0xffff;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += (head->ackNum) & 0xffff;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += ((head->headLen) << 8) + (head->type);
	sum = (sum >> 16) + (sum & 0xffff);
	sum += head->windowSize;
	sum = (sum >> 16) + (sum & 0xffff);
	sum += head->urgentPointer;
	sum = (sum >> 16) + (sum & 0xffff);

	for (int i = 0; i < len - 20; i += 2) {
		sum += ((unsigned short)(head->data[i]) << 8) + ((unsigned short)(head->data[i + 1] & 0xff));
		sum = (sum >> 16) + (sum & 0xffff);
	}

	return (unsigned short)(~sum);
}


int stud_tcp_input(char* pBuffer, unsigned short len, unsigned int srcAddr, unsigned int dstAddr)
{
	TCPHead* head = new TCPHead;
	memcpy(head, pBuffer, len);
	head->ntoh(); // ??????head????????????ntoh data????????????????????????????????????
	// ???????????????
	if (GetCheckSum(head, len, ntohl(srcAddr), ntohl(dstAddr))!= head->checkSum) { // sum not include checkSum, so reverse should == checkSum
		return -1;
	}

	// ???????????????
	if (head->ackNum != curTCB->ackExpect) {
		tcp_DiscardPkt(pBuffer, STUD_TCP_TEST_SEQNO_ERROR);
		return -1;
	}

	// ??????????????????????????? ???????????????CLOSED???SYN_SENT ??? ESTABLISHED ??? FIN_WAIT1 ?????????????????????????????????
	if (curTCB->status == SYN_SENT && ((head->type) & 0x3f) == PACKET_TYPE_SYN_ACK) {
		curTCB->status = ESTABLISHED;
		curTCB->seq = head->ackNum;
		curTCB->ack = head->seqNum + 1;
		stud_tcp_output(NULL, 0, PACKET_TYPE_ACK,
			curTCB->srcPort,
			curTCB->dstPort,
			curTCB->srcAddr,
			curTCB->dstAddr);
		return 0;
	}
	if (curTCB->status == ESTABLISHED) {
		curTCB->seq = head->ackNum;
		curTCB->ack = head->seqNum + (len - (((head->headLen) >> 4) & 0xf) * 4);
		// only reply, no data info is needed
		stud_tcp_output(NULL, 0, PACKET_TYPE_ACK,
			curTCB->srcPort,
			curTCB->dstPort,
			curTCB->srcAddr,
			curTCB->dstAddr);
		return 0;
	}

	if (curTCB->status == FIN_WAIT1 && ((head->type) & 0x3f) == PACKET_TYPE_ACK) {
		curTCB->status = FIN_WAIT2;
		curTCB->seq = head->ackNum;
		curTCB->ack = head->seqNum + 1;
		return 0;
	}
	if (curTCB->status == FIN_WAIT2 && ((head->type) & 0x3f) == PACKET_TYPE_FIN_ACK) {
		curTCB->status = TIME_WAIT;
		stud_tcp_output(NULL, 0, PACKET_TYPE_ACK,
			curTCB->srcPort,
			curTCB->dstPort,
			curTCB->srcAddr,
			curTCB->dstAddr);
		return 0;
	}

	return -1;
}

void stud_tcp_output(char* pData, unsigned short len, unsigned char flag, unsigned short srcPort, unsigned short dstPort, unsigned int srcAddr, unsigned int dstAddr)
{
	if (curTCB == NULL) { // not exec socket??????NULL???
		curTCB = new MyTCB;
		curTCB->ack = gAckNum;
		curTCB->seq = gSeqNum;
		curTCB->srcAddr = srcAddr;
		curTCB->dstAddr = dstAddr;
		curTCB->srcPort = srcPort;
		curTCB->dstPort = dstPort;
		curTCB->status = CLOSED;
	}
	TCPHead* head = new TCPHead;
	memcpy(head->data, pData, len);

	head->srcPort = srcPort;
	head->dstPort = dstPort;
	head->seqNum = curTCB->seq;
	head->ackNum = curTCB->ack;
	head->headLen = 0x50; // 0x50 -> 5 * 4 = 20
	head->type = flag;
	head->windowSize = 1;
	head->urgentPointer = 0;
	head->checkSum = GetCheckSum(head, len + 20, srcAddr, dstAddr); // ???????????????
	head->hton(); //???????????????

	// ????????????????????????
	if (curTCB->status == CLOSED && ((head->type) & 0x3f) == PACKET_TYPE_SYN) {
		curTCB->status = SYN_SENT;
	}
	if (curTCB->status == ESTABLISHED && ((head->type) & 0x3f) == PACKET_TYPE_FIN_ACK) {
		curTCB->status = FIN_WAIT1;
	}

	if (curTCB->status == ESTABLISHED) {
		curTCB->ackExpect = curTCB->seq + len;
	}
	else {
		curTCB->ackExpect = curTCB->seq + 1;
	}
	tcp_sendIpPkt((unsigned char*)head, 20 + len, curTCB->srcAddr, curTCB->dstAddr, 60); // 60 ????????????
}

int stud_tcp_socket(int domain, int type, int protocol)
{
	if (TCBNum == 0) { // ??????sockfd??????1?????? ?????????TCBNum?????????sockfd ??????????????????TCBNum???1??????
		TCBTable.push_back(NULL);
		TCBNum++;
	}
	curTCB = new MyTCB;
	curTCB->ack = gAckNum;
	curTCB->seq = gSeqNum;
	curTCB->srcPort = gSrcPort++; // change srcPort every time
	curTCB->sockfd = TCBNum++;
	curTCB->status = CLOSED;

	TCBTable.push_back(curTCB);
	return curTCB->sockfd;
}

int stud_tcp_connect(int sockfd, struct sockaddr_in* addr, int addrlen)
{
	curTCB = TCBTable[sockfd];

	curTCB->dstPort = ntohs(addr->sin_port);
	curTCB->status = SYN_SENT;
	curTCB->srcAddr = getIpv4Address();
	curTCB->dstAddr = getServerIpv4Address();
	// curTCB->dstAddr = htonl(addr->sin_addr.s_addr);

	stud_tcp_output(NULL, 0, PACKET_TYPE_SYN,
		curTCB->srcPort, curTCB->dstPort,
		curTCB->srcAddr, curTCB->dstAddr);

	// ????????????buffer
	TCPHead* head = new TCPHead;
	int len = waitIpPacket((char*)head, 5000);
	while (len == -1) {
		len = waitIpPacket((char*)head, 5000);
	}
	// ????????????buffer ??????tcp_input
	return stud_tcp_input((char*)head, len, ntohl(curTCB->srcAddr), ntohl(curTCB->dstAddr));
}

int stud_tcp_send(int sockfd, const unsigned char* pData, unsigned short datalen, int flags)
{
	curTCB = TCBTable[sockfd];

	if (curTCB->status == ESTABLISHED) {
		curTCB->data = new char[datalen];
		memcpy(curTCB->data, pData, datalen);
		stud_tcp_output(curTCB->data, datalen, PACKET_TYPE_DATA,
			curTCB->srcPort, curTCB->dstPort,
			curTCB->srcAddr, curTCB->dstAddr);

		// ????????????buffer
		TCPHead* head = new TCPHead;
		int len = waitIpPacket((char*)head, 5000);
		while (len == -1) {
			len = waitIpPacket((char*)head, 5000);
		}
		// ????????????buffer ??????tcp_input
		return stud_tcp_input((char*)head, len, ntohl(curTCB->srcAddr), ntohl(curTCB->dstAddr));
	}
	return -1;
}

int stud_tcp_recv(int sockfd, unsigned char* pData, unsigned short datalen, int flags)
{
	curTCB = TCBTable[sockfd];

	if (curTCB->status == ESTABLISHED) {
		TCPHead* head = new TCPHead;
		int len = waitIpPacket((char*)head, 5000);
		while (len == -1) {
			len = waitIpPacket((char*)head, 5000);
		}
		memcpy(pData, head->data, sizeof(head->data));
		return stud_tcp_input((char*)head, len, ntohl(curTCB->srcAddr), ntohl(curTCB->dstAddr));
	}
	return -1;
}

int stud_tcp_close(int sockfd)
{
	curTCB = TCBTable[sockfd];

	if (curTCB->status == ESTABLISHED) {
		stud_tcp_output(NULL, 0, PACKET_TYPE_FIN_ACK,
			curTCB->srcPort, curTCB->dstPort,
			curTCB->srcAddr, curTCB->dstAddr);

		TCPHead* head = new TCPHead;
		int len = waitIpPacket((char*)head, 5000);
		while (len == -1) {
			len = waitIpPacket((char*)head, 5000);
		}
		stud_tcp_input((char*)head, len, ntohl(curTCB->srcAddr), ntohl(curTCB->dstAddr));

		// ????????????FIN-WAIT2 ?????????????????????FIN????????????TIME-WAIT
		head = new TCPHead;
		len = waitIpPacket((char*)head, 5000);
		while (len == -1) {
			len = waitIpPacket((char*)head, 5000);
		}
		return stud_tcp_input((char*)head, len, ntohl(curTCB->srcAddr), ntohl(curTCB->dstAddr));
	}
	return -1;
}