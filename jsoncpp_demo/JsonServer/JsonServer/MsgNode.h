#pragma once
#include <memory.h>
#include <boost/asio.hpp>

#include "const.h"

/*
tlv应用层协议 头部id + length + value
*/
class MsgNode
{
    friend class CSession;
public:
    MsgNode(const char* msg, short max_len)
        : _total_len(max_len + HEAD_LENGTH), _cur_len(0)
    {
        _data = new char[_total_len + 1]();
        // 转为网络字节序
        short max_len_host = boost::asio::detail::socket_ops::host_to_network_short(max_len);
        memcpy(_data, &max_len_host, HEAD_LENGTH);
        memcpy(_data + HEAD_LENGTH, msg, max_len);
        _data[_total_len] = '\0';
    }

    MsgNode(short max_len) : _total_len(max_len), _cur_len(0)
    {
        _data = new char[_total_len + 1]();
    }

    ~MsgNode()
    {
        delete[] _data;
    }

    void Clear()
    {
        ::memset(_data, 0, _total_len);
        _cur_len = 0;
    }

private:
    short _cur_len;
    short _total_len;
    char* _data;
};

