#include "CSession.h"
#include "CServer.h"
#include "MsgNode.h"

void CSession::Start()
{
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
        std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));

}

std::string& CSession::GetUuid()
{
    return _uuid;
}

void CSession::Send(char* msg, int max_length)
{
    std::lock_guard<std::mutex> lock(_send_lock);
    bool pending = false;
    if (_send_que.size() > 0)
    {
        pending = true;
    }

    _send_que.push(std::make_shared<MsgNode>(msg, max_length));
    if (pending)
        return;

    boost::asio::async_write(_socket, boost::asio::buffer(msg, max_length),
        std::bind(&CSession::HandleWrite, this, std::placeholders::_1, shared_from_this()));
}

// 读数据回调函数
void CSession::HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred, std::shared_ptr<CSession> _self_shared)
{
    if (!error)
    {
        std::cout << "server receive data is " << _data << std::endl;
        Send(_data, bytes_transferred);
        memset(_data, 0, max_length);
        _socket.async_read_some(boost::asio::buffer(_data, max_length),
            std::bind(&CSession::HandleRead, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
    }
    else
    {
        std::cout << "read error" << std::endl;
        _server->ClearSession(_uuid);
    }
}

void CSession::HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared)
{
    if (!error)
    {
        std::lock_guard<std::mutex> lock(_send_lock);
        _send_que.pop();
        if (!_send_que.empty())
        {
            auto& msgNode = _send_que.front();
            boost::asio::async_write(_socket, boost::asio::buffer(msgNode->_data, msgNode->_max_len),
                std::bind(&CSession::HandleWrite, this, std::placeholders::_1, _self_shared));
        }
    }
    else
    {
        std::cout << "write error " << error.value() << std::endl;
        _server->ClearSession(_uuid);
    }
}

