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
������룡��������
�����첽�ص���ʱ����ȷ��
���Ե��û�forѭ��ִ��WriteToSocketErrʱ

�ܿ�����һ��û���������ݻ�δ���ûص�����ʱ�ٴε���WriteToSocketErr
boost::asio��װ��ʱepoll��iocp�ȶ�·����ģ�ͣ���д�¼�������ͷ����ݣ�
���͵����ݰ���async_write_some���õ�˳���ͣ����Իص������ڵ��õ�async_write_some���ܲ�û�б���ʱ���á�


����
//�û���������
WriteToSocketErr("Hello World!");
//�û��޸�֪�²���������һ�η���������
WriteToSocketErr("Hello World!");

��ô�ܿ��ܵ�һ��ֻ������Hello�����������û���꣬�ڶ��η�����Hello World!֮���ַ�����World!
���ԶԶ��յ������ݺܿ����ǡ�HelloHello World! World!��


�������: ͨ�����б�֤���͵�˳����
*/
void Session::WriteCallBackErr(const boost::system::error_code& ec, std::size_t bytes_transferred, std::shared_ptr<MsgNode> msg_node)
{
    // write_some ����һ��û�з�����
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

    // ��������
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
        // �����δ����������ݾ�return
        return;
    }

    // û��Ҫ���͵����ݾ��첽����
    this->_socket->async_write_some(asio::buffer(buf),
        std::bind(&Session::WriteCallBack, this, std::placeholders::_1, std::placeholders::_2));
    _send_pending = true;
}

/*
����������������������
async_send�ڲ����ε��� async_write_some, �޷���֤�ڲ���async_write_some�������Լ���async_write_some��ʱ����
����async_send �� async_write_someѡ��һ��api  һ��ʹ��async_send���һЩ
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

    // ��֤���͵���������
    this->_socket->async_send(asio::buffer(buf),
        std::bind(&Session::WriteAllCallBack, this, std::placeholders::_1, std::placeholders::_2));

    _send_pending = true;
}


/*
ͬ��async_read_some��async_receive���ܻ��ʹ�ã����������߼����⡣

����һ��ʹ��async_read_some���һЩ, ����ÿ�ζ��������� ����������ݺ�Ч��
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

    // �������
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


