#pragma once

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class Session;

class Server
{
public:
    Server(boost::asio::io_context& ioc, short port);

private:
    // ����һ�������� ���ڼ�������
    void start_accept();

    // ��ȡ�����Ļص�����
    void handle_accept(Session* new_session, const boost::system::error_code& error);

private:
    boost::asio::io_context& _ioc;
    tcp::acceptor _acceptor;
};

