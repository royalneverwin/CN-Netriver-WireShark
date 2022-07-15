#include "sysinclude.h"

extern void SendFRAMEPacket(unsigned char* pData, unsigned int len);

#define WINDOW_SIZE_STOP_WAIT 1
#define WINDOW_SIZE_BACK_N_FRAME 4


typedef enum { data, ack, nak } frame_kind;
typedef struct frame_head
{
    frame_kind kind; //帧类型
    unsigned int seq; //序列号
    unsigned int ack; //确认号
    unsigned char data[100];//数据
};
typedef struct frame
{
    frame_head head; //帧头
    unsigned int size; //数据的大小
};

/*
* 注意上面frame 的size 是data大小 不是整个frame大小 所以后面的Buffer和Window需要加一个size记录frame大小
*/

typedef struct Buffer { // 用链表构建缓冲区表示等待发送队列
    frame* message;
    unsigned int size;
    Buffer* nextBuffer;
};

typedef struct Window { // 滑动窗口 用来缓存数据
    frame message;
    unsigned int size;
};


Buffer* AddFrame(Buffer* waitLine, char* pBuffer, int bufferSize) { // 将buffer加到waitLine中
    Buffer* addOne = new Buffer;

    // 构造addOne
    addOne->message = new frame;
    memcpy(addOne->message, pBuffer, bufferSize);
    addOne->size = bufferSize;
    addOne->nextBuffer = NULL;

    // 添加addOne
    if (waitLine == NULL)
        waitLine = addOne;
    else {
        Buffer* tmp = waitLine;
        while (tmp->nextBuffer != NULL)
            tmp = tmp->nextBuffer;
        tmp->nextBuffer = addOne;
    }

    return waitLine;
}

/*
* 停等协议测试函数
*/
int stud_slide_window_stop_and_wait(char* pBuffer, int bufferSize, UINT8 messageType) // pBuffer已经是网络序了
{
    static Window window; // 窗口大小为1
    static Buffer* waitLine = NULL; // 等待队列
    static bool flag = false; // 只有一个窗口 用bool表示是否被使用就行

    int ackRemote;
    int seqLocal;
    switch (messageType) {
    case MSG_TYPE_SEND:
        if (!flag) { // 窗口未被使用 直接发送
            SendFRAMEPacket((unsigned char*)pBuffer, (unsigned int)bufferSize);
            window.message = *((frame*)pBuffer);
            window.size = (unsigned int)bufferSize;
            flag = true;
        }
        else { // 窗口已经被使用 将pBuffer存到waitLine中
            waitLine = AddFrame(waitLine, pBuffer, bufferSize);
        }
        break;


    case MSG_TYPE_RECEIVE:
        ackRemote = ntohl(((frame*)pBuffer)->head.ack);
        seqLocal = ntohl(window.message.head.seq);
        printf("Receive a message, seq is %d\n", ackRemote);
        if (ackRemote == seqLocal) {
            flag = false; // 窗口未被使用
            window.size = 0; // 缓存的不需要了
            if (waitLine != NULL) { // 若等待队列有buffer 就放到窗口里
                Buffer* firstOne = waitLine;
                waitLine = waitLine->nextBuffer;

                SendFRAMEPacket((unsigned char*)(firstOne->message), firstOne->size);
                window.message = *(firstOne->message);
                window.size = firstOne->size;
                flag = true;

                // 释放内存
                delete (firstOne->message);
                delete firstOne;
            }
        }
        break;

    case MSG_TYPE_TIMEOUT:
        SendFRAMEPacket((unsigned char*)&(window.message), window.size); // 重发帧

    }

    return 0;
}



/*
* 回退n帧测试函数
*/
int stud_slide_window_back_n_frame(char* pBuffer, int bufferSize, UINT8 messageType)
{
    static Window window[WINDOW_SIZE_BACK_N_FRAME]; // 窗口大小为WINDOW_SIZE_BACK_N_FRAME
    static Buffer* waitLine = NULL; // 等待队列
    static int start = 0;
    static int end = 0; // 分别指向window的头和尾
    // end指向最后一个buffer之后的位置 start和end使用的时候一直增加 用%WINDOW_SIZE_BACK_N_FRAME表示在window中的index 表示一个环
    int ackRemote;
    int seqLocal;
    switch (messageType) {
    case MSG_TYPE_SEND:
        if (end - start < WINDOW_SIZE_BACK_N_FRAME) { // 窗口未被完全使用 可以发送
            SendFRAMEPacket((unsigned char*)pBuffer, (unsigned int)bufferSize);
            window[end % WINDOW_SIZE_BACK_N_FRAME].message = *((frame*)pBuffer);
            window[end % WINDOW_SIZE_BACK_N_FRAME].size = (unsigned int)bufferSize;
            end++;
        }
        else { // 窗口已经被使用 将pBuffer存到waitLine中
            waitLine = AddFrame(waitLine, pBuffer, bufferSize);
        }
        break;


    case MSG_TYPE_RECEIVE:
        ackRemote = ntohl(((frame*)pBuffer)->head.ack);
        // 和停等协议的区别是要和window中所有的frame的seq比较 删除匹配到的第i个和之前所有的frame
        for (int i = start; i < end; i++) {
            seqLocal = ntohl(window[i % WINDOW_SIZE_BACK_N_FRAME].message.head.seq);
            if (seqLocal == ackRemote) {
                for (int j = start; j <= i; j++) { // 删除前面所有缓存的frame
                    window[j % WINDOW_SIZE_BACK_N_FRAME].size = 0;
                }
                start = i + 1;

                // 如果等待队列有buffer，放到窗口里
                while (end - start < WINDOW_SIZE_BACK_N_FRAME && waitLine != NULL) {
                    Buffer* firstOne = waitLine;
                    waitLine = waitLine->nextBuffer;

                    SendFRAMEPacket((unsigned char*)(firstOne->message), firstOne->size);
                    window[end % WINDOW_SIZE_BACK_N_FRAME].message = *(firstOne->message);
                    window[end % WINDOW_SIZE_BACK_N_FRAME].size = firstOne->size;
                    end++;

                    // 释放内存
                    delete firstOne->message;
                    delete firstOne;
                }

                break;
            }
        }

        break;


    case MSG_TYPE_TIMEOUT:
        // pBuffer指向数据的前四个字节为超时帧的序列号 以UINT32类型存储
        int ackRemote = *(unsigned int*)pBuffer;
        // 测试函数应该将根据帧序号将该帧以及后面发送过的帧重新发送
        for (int i = start; i < end; i++) {
            int seqLocal = ntohl(window[i % WINDOW_SIZE_BACK_N_FRAME].message.head.seq);
            if (seqLocal == ackRemote) {
                for (int j = start; j < end; j++) {
                    SendFRAMEPacket((unsigned char*)&(window[j % WINDOW_SIZE_BACK_N_FRAME].message), window[j % WINDOW_SIZE_BACK_N_FRAME].size);
                }
                break;
            }
        }
    }

    return 0;
}

/*
* 选择性重传测试函数
*/
int stud_slide_window_choice_frame_resend(char* pBuffer, int bufferSize, UINT8 messageType)
{

    static Window window[WINDOW_SIZE_BACK_N_FRAME]; // 窗口大小为WINDOW_SIZE_BACK_N_FRAME
    static Buffer* waitLine = NULL; // 等待队列
    static int start = 0;
    static int end = 0; // 分别指向window的头和尾
    // end指向最后一个buffer之后的位置 start和end使用的时候一直增加 用%WINDOW_SIZE_BACK_N_FRAME表示在window中的index 表示一个环

    int kindRemote;
    int ackRemote;
    switch (messageType) {
    case MSG_TYPE_SEND:
        if (end - start < WINDOW_SIZE_BACK_N_FRAME) { // 窗口未被完全使用 可以发送
            SendFRAMEPacket((unsigned char*)pBuffer, (unsigned int)bufferSize);
            window[end % WINDOW_SIZE_BACK_N_FRAME].message = *((frame*)pBuffer);
            window[end % WINDOW_SIZE_BACK_N_FRAME].size = (unsigned int)bufferSize;
            end++;
        }
        else { // 窗口已经被使用 将pBuffer存到waitLine中
            waitLine = AddFrame(waitLine, pBuffer, bufferSize);
        }
        break;


    case MSG_TYPE_RECEIVE:
        kindRemote = ntohl(((frame*)pBuffer)->head.kind);
        ackRemote = ntohl(((frame*)pBuffer)->head.ack);
        if (kindRemote == ack) {
            // 和停等协议的区别是要和window中所有的frame的seq比较 删除匹配到的第i个和之前所有的frame
            for (int i = start; i < end; i++) {
                int seqLocal = ntohl(window[i % WINDOW_SIZE_BACK_N_FRAME].message.head.seq);
                if (seqLocal == ackRemote) {
                    for (int j = start; j <= i; j++) { // 删除前面所有缓存的frame
                        window[j % WINDOW_SIZE_BACK_N_FRAME].size = 0;
                    }
                    start = i + 1;

                    // 如果等待队列有buffer，放到窗口里
                    while (end - start < WINDOW_SIZE_BACK_N_FRAME && waitLine != NULL) {
                        Buffer* firstOne = waitLine;
                        waitLine = waitLine->nextBuffer;

                        SendFRAMEPacket((unsigned char*)(firstOne->message), firstOne->size);
                        window[end % WINDOW_SIZE_BACK_N_FRAME].message = *(firstOne->message);
                        window[end % WINDOW_SIZE_BACK_N_FRAME].size = firstOne->size;
                        end++;

                        // 释放内存
                        delete firstOne->message;
                        delete firstOne;
                    }

                    break;
                }
            }
        }
        else if (kindRemote == nak) {
            for (int i = start; i < end; i++) {
                int seqLocal = ntohl(window[i % WINDOW_SIZE_BACK_N_FRAME].message.head.seq);
                if (seqLocal == ackRemote) {
                    SendFRAMEPacket((unsigned char*)&(window[i % WINDOW_SIZE_BACK_N_FRAME].message), window[i % WINDOW_SIZE_BACK_N_FRAME].size);
                    break;
                }
            }
        }
        else {
            printf("something wrong happened\n");
        }
        break;



    case MSG_TYPE_TIMEOUT:
        // pBuffer指向数据的前四个字节为超时帧的序列号 以UINT32类型存储
        int ackRemote = *(unsigned int*)pBuffer;
        // 测试函数应该将根据帧序号将该帧以及后面发送过的帧重新发送
        for (int i = start; i < end; i++) {
            int seqLocal = ntohl(window[i % WINDOW_SIZE_BACK_N_FRAME].message.head.seq);
            if (seqLocal == ackRemote) {
                for (int j = start; j < end; j++) {
                    SendFRAMEPacket((unsigned char*)&(window[j % WINDOW_SIZE_BACK_N_FRAME].message), window[j % WINDOW_SIZE_BACK_N_FRAME].size);
                }
                break;
            }
        }
    }
    return 0;
}

