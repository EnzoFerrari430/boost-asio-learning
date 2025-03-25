#include "MsgNode.h"
#include <memory.h>

MsgNode::MsgNode(const char* msg, int total_len)
    : _total_len(total_len), _cur_len(0)
{
    _msg = new char[total_len];
    memcpy(_msg, msg, total_len);
}

MsgNode::MsgNode(int total_len)
    : _total_len(total_len), _cur_len(0)
{
    _msg = new char[total_len];
}

MsgNode::~MsgNode()
{
    if (_msg)
        delete[] _msg;
}
