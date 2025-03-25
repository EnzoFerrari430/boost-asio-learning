#include "Session.h"

Session::Session(std::shared_ptr<asio::ip::tcp::socket> socket)
    : _socket(socket)
    , _send_pending(false)
    , _recv_pending(false)
{

}

void Session::Connect(const asio::ip::tcp::endpoint& ep)
{
    _socket->connect(ep);
}

/*
问题代码！！！！！
由于异步回调的时机不确定
所以当用户for循环执行WriteToSocketErr时

很可能在一次没发送完数据还未调用回调函数时再次调用WriteToSocketErr
boost::asio封装的时epoll和iocp等多路复用模型，当写事件就绪后就发数据，
发送的数据按照async_write_some调用的顺序发送，所以回调函数内调用的async_write_some可能并没有被及时调用。


例如
//用户发送数据
WriteToSocketErr("Hello World!");
//用户无感知下层调用情况又一次发送了数据
WriteToSocketErr("Hello World!");

那么很可能第一次只发送了Hello，后面的数据没发完，第二次发送了Hello World!之后又发送了World!
所以对端收到的数据很可能是”HelloHello World! World!”


解决方案: 通过队列保证发送的顺序性
*/
void Session::WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<MsgNode> msg_node)
{
    // write_some 可能一次没有发送完
    if (bytes_transferred + msg_node->_cur_len < msg_node->_total_len)
    {
        _send_node->_cur_len += bytes_transferred;
        this->_socket->async_write_some(asio::buffer(_send_node->_msg + _send_node->_cur_len, _send_node->_total_len - _send_node->_cur_len),
            std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, _send_node));
    }
}

void Session::WriteToSocketErr(const std::string& buf)
{
    _send_node = std::make_shared<MsgNode>(buf.c_str(), buf.length());
    _socket->async_write_some(asio::buffer(_send_node->_msg, _send_node->_total_len),
        std::bind(&Session::WriteCallBackErr, this, std::placeholders::_1, std::placeholders::_2, _send_node));
}



void Session::WriteCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    if (ec.value() != 0)
    {
        std::cout << "Error, code is " << ec.value() << " .Message is" << ec.message() << std::endl;
        return;
    }

    auto& send_data = _send_queue.front();
    send_data->_cur_len += bytes_transferred;
    if (send_data->_cur_len < send_data->_total_len) {
        this->_socket->async_write_some(asio::buffer(send_data->_msg + send_data->_cur_len, send_data->_total_len - send_data->_cur_len),
            std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
        return;
    }

    // 发送完了
    _send_queue.pop();
    if (_send_queue.empty())
    {
        _send_pending = false;
    }

    if (!_send_queue.empty())
    {
        auto& send_data = _send_queue.front();
        this->_socket->async_write_some(asio::buffer(send_data->_msg + send_data->_cur_len, send_data->_total_len - send_data->_cur_len),
            std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));

    }

}

void Session::WriteToSocket(const std::string& buf)
{
    _send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
    if (_send_pending)
    {
        // 如果有未发送完的数据就return
        return;
    }

    // 没有要发送的数据就异步发送
    this->_socket->async_write_some(asio::buffer(buf),
        std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _send_pending = true;
}

/*
！！！！！！！！！！！
async_send内部会多次调用 async_write_some, 无法保证内部的async_write_some和我们自己的async_write_some的时序性
所以async_send 和 async_write_some选择一个api  一般使用async_send会多一些
*/
void Session::WriteAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    if (ec.value() != 0)
    {
        std::cout << "Error, code is " << ec.value() << " .Message is" << ec.message() << std::endl;
        return;
    }

    _send_queue.pop();
    if (_send_queue.empty())
    {
        _send_pending = false;
    }

    if (!_send_queue.empty())
    {
        auto& send_data = _send_queue.front();
        this->_socket->async_send(asio::buffer(send_data->_msg + send_data->_cur_len, send_data->_total_len - send_data->_cur_len),
            std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));

    }
}

void Session::WriteAllToSocket(const std::string& buf)
{
    _send_queue.emplace(new MsgNode(buf.c_str(), buf.length()));
    if (_send_pending)
        return;

    // 保证发送的数据完整
    this->_socket->async_send(asio::buffer(buf),
        std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));

    _send_pending = true;
}


/*
同样async_read_some和async_receive不能混合使用，否则会出现逻辑问题。

这里一般使用async_read_some会多一些, 尽量每次都接收数据 方便管理数据和效率
*/
void Session::ReadCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    _recv_node->_cur_len += bytes_transferred;
    if (_recv_node->_cur_len < _recv_node->_total_len)
    {
        _socket->async_read_some(asio::buffer(_recv_node->_msg + _recv_node->_cur_len, _recv_node->_total_len - _recv_node->_cur_len),
            std::bind(&Session::ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));
        return;
    }

    // 接收完毕
    _recv_pending = false;
    _recv_node = nullptr;
}

void Session::ReadFromSocket()
{
    if (_recv_pending)
        return;

    _recv_node = std::make_shared<MsgNode>(RECVSIZE);
    _socket->async_read_some(asio::buffer(_recv_node->_msg, _recv_node->_total_len),
        std::bind(&Session::ReadCallBack, this, std::placeholders::_1, std::placeholders::_2));

    _recv_pending = true;
}

void Session::ReadAllCallBack(const boost::system::error_code& ec, std::size_t bytes_transferred)
{
    _recv_node->_cur_len += bytes_transferred;
    _recv_node = nullptr;
    _recv_pending = false;

}

void Session::ReadAllFromSocket()
{
    if (_recv_pending)
        return;

    _recv_node = std::make_shared<MsgNode>(RECVSIZE);
    _socket->async_receive(asio::buffer(_recv_node->_msg, _recv_node->_total_len),
        std::bind(&Session::ReadAllCallBack, this, std::placeholders::_1, std::placeholders::_2));

    _recv_pending = true;
}


