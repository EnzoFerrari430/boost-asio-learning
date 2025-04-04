#pragma once

#include <memory>
#include <map>
#include <mutex>

#include <boost/asio.hpp>

#include "CSession.h"

class CServer
{
public:
    CServer(boost::asio::io_context& io_context, short port);
    ~CServer();

    void ClearSession(const std::string& uuid);
private:
    void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code& error);
    void StartAccept();
private:
    boost::asio::io_context& _io_context;
    short _port;
    boost::asio::ip::tcp::acceptor _acceptor;
    std::map<std::string, std::shared_ptr<CSession>> _sessions;
    std::mutex _mutex;
};

