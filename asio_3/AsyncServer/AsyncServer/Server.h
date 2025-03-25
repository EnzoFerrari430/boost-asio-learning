#pragma once
#include <map>

#include <boost/asio.hpp>
using boost::asio::ip::tcp;

class Session;

class Server
{
public:
    Server(boost::asio::io_context& ioc, short port);

    void ClearSession(const std::string& uuid);
private:
    // ����һ�������� ���ڼ�������
    void start_accept();

    // ��ȡ�����Ļص�����
    void handle_accept(std::shared_ptr<Session> new_session, const boost::system::error_code& error);

private:
    boost::asio::io_context& _ioc;
    tcp::acceptor _acceptor;


    std::map<std::string, std::shared_ptr<Session>> _sessions;
};

