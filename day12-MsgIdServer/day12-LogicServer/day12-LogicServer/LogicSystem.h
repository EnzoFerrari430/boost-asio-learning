#pragma once

#include <queue>
#include <thread>
#include <map>
#include <functional>

#include <json/json.h>
#include <json/value.h>
#include <json/reader.h>

#include "Singleton.h"
#include "CSession.h"
#include "const.h"

typedef std::function<void(std::shared_ptr<CSession>, const short& msg_id, const std::string& msg_data)> FunCallBack;

class LogicSystem : public Singleton<LogicSystem>
{
    // Singleton要调用LogicSystem的构造函数所以要友元
    friend class Singleton<LogicSystem>;
public:
    ~LogicSystem();

    void PostMsgToQue(std::shared_ptr<LogicNode> msg);
private:
    LogicSystem();

    // 注册回调
    void RegisterCallBacks();
    void HelloWorldCallBack(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data);
    void DealMsg();

private:
    std::queue<std::shared_ptr<LogicNode>> _msg_que;
    std::mutex _mutex;
    std::condition_variable _consume; // 条件变量
    std::thread _worker_thread;
    bool _b_stop;
    std::map<short, FunCallBack> _fun_callback;
};

