#include "LogicSystem.h"

LogicSystem::LogicSystem()
    : _b_stop(false)
{
    RegisterCallBacks();
    _worker_thread = std::thread(&LogicSystem::DealMsg, this);
}

LogicSystem::~LogicSystem()
{
    _b_stop = true;
    _consume.notify_one();
    _worker_thread.join();
}

LogicSystem& LogicSystem::GetInstance()
{
    static LogicSystem instance;
    return instance;
}

void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
    std::unique_lock<std::mutex> unique_lock(_mutex);
    _msg_que.push(msg);
    if (_msg_que.size() == 1)
    {
        _consume.notify_one();
    }
}

// 处理消息线程
void LogicSystem::DealMsg()
{
    for (;;)
    {
        std::unique_lock<std::mutex> unique_lock(_mutex);
        while (_msg_que.empty() && !_b_stop)
        {
            _consume.wait(unique_lock);
        }

        if (_b_stop)
        {
            while (!_msg_que.empty())
            {
                auto msg_node = _msg_que.front();
                std::cout << "recv msg id is " << msg_node->_recvnode->_msg_id << std::endl;
                auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
                if (call_back_iter == _fun_callbacks.end())
                {
                    _msg_que.pop();
                    continue;
                }

                call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
                    std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_total_len));
                _msg_que.pop();
            }

            break; //退出线程
        }

        // 如果没有停服，并且队列中有数据
        auto msg_node = _msg_que.front();
        std::cout << "recv msg id is " << msg_node->_recvnode->_msg_id << std::endl;
        auto call_back_iter = _fun_callbacks.find(msg_node->_recvnode->_msg_id);
        if (call_back_iter == _fun_callbacks.end())
        {
            // 没找到 不处理
            _msg_que.pop();
            continue;
        }

        // 处理
        call_back_iter->second(msg_node->_session, msg_node->_recvnode->_msg_id,
            std::string(msg_node->_recvnode->_data, msg_node->_recvnode->_total_len));
        _msg_que.pop();

    }
}

void LogicSystem::RegisterCallBacks()
{
    _fun_callbacks[MSG_HELLO_WORLD] = std::bind(&LogicSystem::HelloWorldCallBacks, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::HelloWorldCallBacks(std::shared_ptr<CSession> session, short msg_id, const std::string& msg_data)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    std::cout << "receive msg id is " << root["id"].asInt() << " msg data is " << root["data"].asString() << std::endl;
    root["data"] = "server has received msg, msg data is " + root["data"].asString();

    std::string return_str = root.toStyledString();
    session->Send(return_str, root["id"].asInt());
}
