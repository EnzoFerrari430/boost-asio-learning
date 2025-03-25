#pragma once

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class Session;

class Server
{
public:
    Server(boost::asio::io_context& ioc, short port);

private:
    // 启动一个描述符 用于监听链接
    void start_accept();

    // 获取监听的回调函数
    void handle_accept(Session* new_session, const boost::system::error_code& error);

private:
    boost::asio::io_context& _ioc;
    tcp::acceptor _acceptor;
};

