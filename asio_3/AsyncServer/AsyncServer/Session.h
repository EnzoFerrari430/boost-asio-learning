#pragma once

#include <iostream>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class Session
{
public:
    Session(boost::asio::io_context& ioc) : _socket(ioc) {

    }

    tcp::socket& Socket() { return _socket; }

    // 在start函数里监听对客户端的读和写
    void Start();

private:
    // echo类型的服务器
    void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error);

private:
    tcp::socket _socket;
    enum {max_length = 1024};
    char _data[max_length];
};

