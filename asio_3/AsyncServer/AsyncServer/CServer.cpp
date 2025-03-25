﻿#include "CServer.h"
#include "CSession.h"


CServer::CServer(boost::asio::io_context& ioc, short port)
    : _ioc(ioc)
    , _acceptor(ioc, tcp::endpoint(tcp::v4(), port))
{
    std::cout << "Server start success, on port: " << port << std::endl;
    start_accept();
}

void CServer::ClearSession(const std::string& uuid)
{
    _sessions.erase(uuid);
}

// 启动一个描述符 用于监听链接
void CServer::start_accept()
{
    std::shared_ptr<CSession> new_session = std::make_shared<CSession>(_ioc, this);
    _acceptor.async_accept(new_session->Socket(), std::bind(&CServer::handle_accept, this, new_session, std::placeholders::_1));
}

void CServer::handle_accept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error)
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
