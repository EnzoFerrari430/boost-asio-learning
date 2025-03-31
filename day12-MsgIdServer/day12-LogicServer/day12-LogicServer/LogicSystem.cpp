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
    _consume.notify_one();//唤醒工作线程
    _worker_thread.join();//等待工作线程退出
}

// 外部投递消息接口
void LogicSystem::PostMsgToQue(std::shared_ptr<LogicNode> msg)
{
    std::unique_lock<std::mutex> unique_lock(_mutex);
    _msg_que.push(msg);

    // 消息队列从0-1 唤醒逻辑处理线程
    if (_msg_que.size() == 1)
    {
        _consume.notify_one();
    }
}

// 注册回调函数
void LogicSystem::RegisterCallBacks()
{
    _fun_callback[MSG_HELLO_WORLD] = std::bind(&LogicSystem::HelloWorldCallBack,
        this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}

void LogicSystem::HelloWorldCallBack(std::shared_ptr<CSession> session, const short& msg_id, const std::string& msg_data)
{
    Json::Reader reader;
    Json::Value root;
    reader.parse(msg_data, root);
    std::cout << "receive msg id is " << root["id"].asInt() << " msg data is " << root["data"].asString() << std::endl;
    root["data"] = "server has receive msg, msg data is " + root["data"].asString();

    std::string return_str = root.toStyledString();
    session->Send(return_str, root["id"].asInt());
}

// 逻辑处理线程
void LogicSystem::DealMsg()
{
    for (;;)
    {
        // unique_lock可以使用条件变量随时解锁
        std::unique_lock<std::mutex> unique_lock(_mutex);

        while (_msg_que.empty() && !_b_stop)
        {
            _consume.wait(unique_lock);
        }

        // 如果是关闭状态， 取出逻辑队列所有数据及时处理并退出循环
        if (_b_stop)
        {
            while (!_msg_que.empty())
            {
                auto msg_node = _msg_que.front();
                std::cout << "recv msg id is " << msg_node->_recvnode->_msg_id << std::endl;
                auto call_back_iter = _fun_callback.find(msg_node->_recvnode->_msg_id);
                if (call_back_iter == _fun_callback.end())
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

            // 处理完LogicNode 退出逻辑线程
            break;
        }

        // 如果没有停服，并且队列中有数据
        auto msg_node = _msg_que.front();
        std::cout << "recv msg id is " << msg_node->_recvnode->_msg_id << std::endl;
        auto call_back_iter = _fun_callback.find(msg_node->_recvnode->_msg_id);
        if (call_back_iter == _fun_callback.end())
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
