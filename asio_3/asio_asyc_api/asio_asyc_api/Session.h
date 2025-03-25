#pragma once


/*
asio 异步读写的api

*/

#include <memory>
#include <boost/asio.hpp>
#include <iostream>
#include <queue>

#include "MsgNode.h"

using namespace boost;

// 会话类
class Session
{
public:
    Session(std::shared_ptr<asio::ip::tcp::socket> socket);
    void Connect(const asio::ip::tcp::endpoint& ep);

    //异步写操作 这种写法有点问题 所以函数名后面加上了Err
    void WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred,
        std::shared_ptr<MsgNode> msg_node);
    void WriteToSocketErr(const std::string& buf);

    // 通过队列实现顺序读写
    // use async_write_some API
    void WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void WriteToSocket(const std::string& buf);

    // use async_send API
    void WriteAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void WriteAllToSocket(const std::string& buf);


    // 异步读操作
    // use async_read_some API
    void ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void ReadFromSocket();

    void ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred);
    void ReadAllFromSocket();
private:
    // 消息队列保证发送的顺序性
    std::queue<std::shared_ptr<MsgNode>> _send_queue;
    std::shared_ptr<asio::ip::tcp::socket> _socket;
    std::shared_ptr<MsgNode> _send_node; // 写结点
    bool _send_pending;

    std::shared_ptr<MsgNode> _recv_node;
    bool _recv_pending;
};

