#include "ConnectionMgr.h"

ConnectionMgr::ConnectionMgr()
{
}

ConnectionMgr& ConnectionMgr::GetInstance()
{
    static ConnectionMgr instance;
    return instance;
}

void ConnectionMgr::AddConnection(std::shared_ptr<Connection> conptr)
{
    _map_cons.emplace(std::make_pair(conptr->GetUid(), conptr));
}

void ConnectionMgr::RmvConnection(const std::string& id)
{
    _map_cons.erase(id);
}

