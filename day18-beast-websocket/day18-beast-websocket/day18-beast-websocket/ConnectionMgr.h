#pragma once

#include <boost/unordered_map.hpp>

#include "Connection.h"

class ConnectionMgr
{
public:
    static ConnectionMgr& GetInstance();

    void AddConnection(std::shared_ptr<Connection> conptr);
    void RmvConnection(const std::string& id);

private:
    ConnectionMgr(const ConnectionMgr&) = delete;
    ConnectionMgr& operator=(const ConnectionMgr&) = delete;

    ConnectionMgr();

    boost::unordered_map<std::string, std::shared_ptr<Connection>> _map_cons;

};

