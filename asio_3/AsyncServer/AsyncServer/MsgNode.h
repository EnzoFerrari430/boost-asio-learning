#pragma once
#include <memory.h>
class MsgNode
{
    friend class CSession;
public:
    MsgNode(char* msg, int max_len)
        : _max_len(max_len), _cur_len(0)
    {
        _data = new char[max_len];
        memcpy(_data, msg, max_len);
    }

    ~MsgNode()
    {
        delete[] _data;
    }
private:
    int _cur_len;
    int _max_len;
    char* _data;
};

