#include "Server.h"
#include "Session.h"


Server::Server(boost::asio::io_context& ioc, short port)
    : _ioc(ioc)
    , _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server start success, on port: " << port << std::endl;
    start_accept();
}

void Server::ClearSession(const std::string& uuid)
{
    _sessions.erase(uuid);
}

// 启动一个描述符 用于监听链接
void Server::start_accept()
{
    std::shared_ptr<Session> new_session = std::make_shared<Session>(_ioc, this);
    _acceptor.async_accept(new_session->Socket(), std::bind(&Server::handle_accept, this, new_session, std::placeholders::_1));
}

void Server::handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->Start();
        _sessions.insert(std::make_pair(new_session->GetUuid(), new_session));
    }
    else
    {
        //delete new_session;
    }
    start_accept();
}
