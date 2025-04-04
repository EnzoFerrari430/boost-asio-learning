#include "CServer.h"
#include "AsioIOServicePool.h"

#include <iostream>

CServer::CServer(boost::asio::io_context& io_context, short port)
    : _io_context(io_context), _port(port), _acceptor(io_context, tcp::endpoint(tcp::v4(), port))
{
    StartAccept();
}

CServer::~CServer()
{

}

void CServer::ClearSession(const std::string& uuid)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _sessions.erase(uuid);
}

void CServer::StartAccept()
{
    auto& ioc = AsioIOServicePool::GetInstance().GetIOService();
    std::shared_ptr<CSession> new_session = std::make_shared<CSession>(ioc, this);
    _acceptor.async_accept(new_session->GetSocket(),
        std::bind(&CServer::HandleAccept, this, new_session, std::placeholders::_1));
}

void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error)
{
    if (!error)
    {
        new_session->Start();
        std::lock_guard<std::mutex> lock(_mutex);
        _sessions.insert(std::make_pair(new_session->GetUuid(), new_session));
    }
    else
    {
        std::cout << "session accept failed, error is " << error.what() << std::endl;
    }

    StartAccept();
}


