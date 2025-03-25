#pragma once
#include <map>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class Session;

class Server
{
public:
    Server(boost::asio::io_context& ioc, short port);

    void ClearSession(const std::string& uuid);
private:
    // 启动一个描述符 用于监听链接
    void start_accept();

    // 获取监听的回调函数
    void handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& error);

private:
    boost::asio::io_context& _ioc;
    tcp::acceptor _acceptor;


    std::map<std::string, std::shared_ptr<Session>> _sessions;
};

