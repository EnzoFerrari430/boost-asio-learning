#pragma once


const int RECVSIZE = 1024;

/*
MsgNode类 用于管理要发送和接收的数据

有三个成员变量：消息的首地址 数据总长度 当前以处理长度

要发送的数据就构造这样的结构去发送
*/
class MsgNode
{
    friend class Session;
public:
    // 写结点
    MsgNode(const char* msg, int total_len);
    // 读结点
    MsgNode(int total_len);
    ~MsgNode();
private:
    char* _msg{ nullptr };
    int _total_len;
    int _cur_len;
};

