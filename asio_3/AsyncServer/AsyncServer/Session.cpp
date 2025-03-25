#include "Session.h"
#include "Server.h"

void Session::Start()
{
    memset(_data, 0, max_length);
    _socket.async_read_some(boost::asio::buffer(_data, max_length),
        std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, shared_from_this()));

}

std::string& Session::GetUuid()
{
    return _uuid;
}

// 读数据回调函数
void Session::handle_read(const boost::system::error_code& error, std::size_t bytes_transferred, std::shared_ptr<Session> _self_shared)
{
    if (!error)
    {
        std::cout << "server receive data is " << _data << std::endl;

        memset(_data, 0, max_length);
        _socket.async_read_some(boost::asio::buffer(_data, max_length),
            std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));

        boost::asio::async_write(_socket, boost::asio::buffer("hello client"/*_data, bytes_transferred*/),
            std::bind(&Session::handle_write, this, std::placeholders::_1, _self_shared));
    }
    else
    {
        std::cout << "read error" << std::endl;
        //delete this; // ???
        _server->ClearSession(_uuid);
    }
}

void Session::handle_write(const boost::system::error_code& error, std::shared_ptr<Session> _self_shared)
{
    if (!error)
    {
        memset(_data, 0, max_length);
        _socket.async_read_some(boost::asio::buffer(_data, max_length),
            std::bind(&Session::handle_read, this, std::placeholders::_1, std::placeholders::_2, _self_shared));
    }
    else
    {
        std::cout << "write error " << error.value() << std::endl;
        //delete this; // ???
        _server->ClearSession(_uuid);
    }
}
/*
该demo示例为仿照asio官网编写的，其中存在隐患，
就是当服务器即将发送数据前(调用async_write前)，此刻客户端中断，服务器此时调用async_write会触发发送回调函数，判断ec为非0进而执行delete this逻辑回收session。
但要注意的是客户端关闭后，在tcp层面会触发读就绪事件，服务器会触发读事件回调函数。
在读事件回调函数中判断错误码ec为非0，进而再次执行delete操作，从而造成二次析构，这是极度危险的。
*/
