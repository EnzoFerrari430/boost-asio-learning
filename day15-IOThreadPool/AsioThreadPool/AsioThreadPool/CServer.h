#pragma once
#include <map>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class CSession;

class CServer
{
public:
    CServer(boost::asio::io_context& ioc, short port);

    void ClearSession(const std::string& uuid);
private:
    // 启动一个描述符 用于监听链接
    void StartAccept();

    // 获取监听的回调函数
    void HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error);

private:
    boost::asio::io_context& _ioc;
    tcp::acceptor _acceptor;


    std::map<std::string, std::shared_ptr<CSession>> _sessions;
};

