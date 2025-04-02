﻿#pragma once
#include <memory.h>
#include <iostream>
#include <boost/asio.hpp>

#include "const.h"

class LogicSystem;

/*
tlv应用层协议 头部id + length + value

这里对MsgNode进行分类  分成RecvNode和SendNode
*/
class MsgNode
{
public:
    MsgNode(short max_len)
        : _total_len(max_len), _cur_len(0)
    {
        _data = new char[_total_len + 1]();
        _data[_total_len] = '\0';
    }

    ~MsgNode()
    {
        std::cout << "destruct MsgNode" << std::endl;
        delete[] _data;
    }

    void Clear()
    {
        ::memset(_data, 0, _total_len);
        _cur_len = 0;
    }

    short _cur_len;
    short _total_len;
    char* _data;
};

class RecvNode : public MsgNode
{
    friend class LogicSystem;
public:
    RecvNode(short max_len, short msg_id);
private:
    short _msg_id;
};

class SendNode : public MsgNode
{
    friend class LogicSystem;
public:
    SendNode(const char* msg, short max_len, short msg_id);
private:
    short _msg_id;
};

