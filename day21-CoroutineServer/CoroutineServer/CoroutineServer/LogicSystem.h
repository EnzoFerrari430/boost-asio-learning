#pragma once

#include <queue>
#include <thread>
#include <map>
#include <functional>

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#include "const.h"
#include "CSession.h"

typedef std::function<void(std::shared_ptr<CSession>, short msg_id, const std::string& msg_data)> FunCallBack;

class LogicSystem
{
public:
    ~LogicSystem();

    void PostMsgToQue(std::shared_ptr<LogicNode> msg);
    static LogicSystem& GetInstance();

    LogicSystem(const LogicSystem&) = delete;
    LogicSystem& operator=(const LogicSystem&) = delete;
private:
    LogicSystem();

    void DealMsg();
    void RegisterCallBacks();
    void HelloWorldCallBacks(std::shared_ptr<CSession> session, short msg_id, const std::string& msg_data);

private:
    std::thread _worker_thread;
    std::queue<std::shared_ptr<LogicNode>> _msg_que;
    std::mutex _mutex;
    std::condition_variable _consume;
    bool _b_stop;
    std::map<short, FunCallBack> _fun_callbacks;
};

