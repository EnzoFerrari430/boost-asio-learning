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

// �����ݻص�����
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
��demoʾ��Ϊ����asio������д�ģ����д���������
���ǵ�������������������ǰ(����async_writeǰ)���˿̿ͻ����жϣ���������ʱ����async_write�ᴥ�����ͻص��������ж�ecΪ��0����ִ��delete this�߼�����session��
��Ҫע����ǿͻ��˹رպ���tcp����ᴥ���������¼����������ᴥ�����¼��ص�������
�ڶ��¼��ص��������жϴ�����ecΪ��0�������ٴ�ִ��delete�������Ӷ���ɶ������������Ǽ���Σ�յġ�
*/
