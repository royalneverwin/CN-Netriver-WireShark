/*
* THIS FILE IS FOR IP FORWARD TEST
*/
#include "sysInclude.h"
#include <vector>
#include <algorithm>
#include <iostream>
using namespace std;

// system support
extern void fwd_LocalRcv(char *pBuffer, int length);

extern void fwd_SendtoLower(char *pBuffer, int length, unsigned int nexthop);

extern void fwd_DiscardPkt(char *pBuffer, int type);

extern unsigned int getIpv4Address( );

// implemented by students


// vector查找 因为要按照最长匹配查找 所以要遍历vector
typedef struct Node{
    unsigned int dest;
    unsigned int maskLen;
    unsigned int mask;// 子网掩码：为1的部分表示子网地址，为0的部分表示主机地址
    unsigned int nexthop;
    Node(unsigned int dt, unsigned int ml, unsigned int mk,unsigned int nh){
        dest = dt;
        maskLen = ml;
        mask = mk;
        nexthop = nh;
    }
};

vector<Node> RouteList;

void stud_Route_Init()
{
    RouteList.clear();
    return;
}

void stud_route_add(stud_route_msg *proute)
{
    // 转化成主机序
    unsigned int dest = ntohl(proute->dest);
    unsigned int masklen = ntohl(proute->masklen);
    unsigned int nexthop = ntohl(proute->nexthop);

    cout << "maskLen: " << masklen << endl;
    unsigned int mask = (1<<31)>>(masklen-1);
    cout << ((mask >> 24) & 0xff) << ',' << ((mask >> 16) & 0xff) << ',' << ((mask >> 8) & 0xff) << ',' << ((mask) & 0xff) << endl;


    Node tmp(dest, masklen, mask, nexthop);
    RouteList.push_back(tmp);
    return;
}


int stud_fwd_deal(char *pBuffer, int length)  // 已完成IP报文的合法性检查
{
    unsigned int headLen = (ntohl(((unsigned int *)pBuffer)[0]) >> 24) & 0xf;

    vector<unsigned int> bufferHead(headLen, 0);
    for (int i = 0; i < headLen; i++) {
        bufferHead[i] = ntohl(((unsigned int *)pBuffer)[i]);
    }

    unsigned int dstAddress = bufferHead[4];
    char TTL = char((bufferHead[2] >> 24) & 0xff);



    cout << "begin TTL: " << int(TTL) << endl;
    for(int i = 0; i < RouteList.size(); i++){
        cout << ((RouteList[i].dest >> 24) & 0xff) << ',' << ((RouteList[i].dest >> 16) & 0xff) << ',' << ((RouteList[i].dest >> 8) & 0xff) << ',' << ((RouteList[i].dest) & 0xff) << ' ';
        cout << RouteList[i].maskLen << ' ';
        cout << ((RouteList[i].mask >> 24) & 0xff) << ',' << ((RouteList[i].mask >> 16) & 0xff) << ',' << ((RouteList[i].mask >> 8) & 0xff) << ',' << ((RouteList[i].mask) & 0xff) << endl;
    }
    if (TTL <= 0) { // TTL < 0
        fwd_DiscardPkt(pBuffer, STUD_FORWARD_TEST_TTLERROR);
        return 1;
    } 


    if (dstAddress != getIpv4Address()) { // 目的地址不是本路由器
        unsigned int maxLen = 0;
        unsigned int nexthop = 0;
        bool flag = false;
        for(int i = 0; i < RouteList.size(); i++){
            if(RouteList[i].maskLen > maxLen && (dstAddress & RouteList[i].mask) == RouteList[i].dest){
                maxLen = RouteList[i].maskLen;
                nexthop = RouteList[i].nexthop;
                flag = true;
            }
        }
        
        cout << "flag: " << flag << endl;
        if(!flag){ // 没有匹配到
            fwd_DiscardPkt(pBuffer, STUD_FORWARD_TEST_NOROUTE);
            return 1;               
        }

        // 匹配到了
        TTL -= 1;
        cout << "TTL: " << TTL << endl;
        bufferHead[2] = (bufferHead[2] & 0x00ff0000) | ((unsigned int)TTL << 24);
        // 计算检验和
        unsigned int sum = 0;
        for (int i = 0; i < headLen; i++) {
            sum += (bufferHead[i] >> 16) & 0xffff; // calculate the sum
            sum = (sum >> 16) + (sum & 0xffff); // wrap around when overflow
            sum += bufferHead[i] & 0xffff; // calculate the sum
            sum = (sum >> 16) + (sum & 0xffff); // wrap around when overflow
        }

        sum = ~sum;
        bufferHead[2] += (unsigned short)sum;

            // 对首部要全部转成网络序
        for (int i = 0; i < headLen; i++) {
            bufferHead[i] = htonl(bufferHead[i]);
            memcpy(&(((unsigned int *)pBuffer)[i]), &(bufferHead[i]), sizeof(unsigned int));
        }

        fwd_SendtoLower(pBuffer, length, nexthop);
    }
    else{
        fwd_LocalRcv(pBuffer, length);
    }

    return 0;
}
