#pragma once
#include <memory.h>

#define MAX_LENGTH 1024*2
#define HEAD_LENGTH 2
/*
tlv应用层协议 头部id + length + value
*/
class MsgNode
{
    friend class CSession;
public:
    MsgNode(char* msg, short max_len)
        : _total_len(max_len + HEAD_LENGTH), _cur_len(0)
    {
        _data = new char[_total_len + 1]();
        memcpy(_data, &max_len, HEAD_LENGTH);
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

