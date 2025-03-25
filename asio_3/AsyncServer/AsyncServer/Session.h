#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
using boost::asio::ip::tcp;

class Server;

/*
std::enable_shared_from_this �� C++ ��׼���е�һ��ģ���࣬���� <memory> ͷ�ļ��ж��塣
����Ҫ��������һ�������ܹ���ȫ�ش����ڲ���ȡһ��ָ������� std::shared_ptr��

��ʹ�� std::shared_ptr ���������������ʱ����ʱ����������Ҫ����һ��ָ���Լ��� std::shared_ptr.
��ֱ��ʹ�� this ָ�봴�� std::shared_ptr ���������⣬
��Ϊ��������������������ͬһ����� std::shared_ptr���Ӷ����ܵ��¶��󱻶��ɾ��������δ������Ϊ��

Ҫʹ�� std::enable_shared_from_this����Ҫ����̳��� std::enable_shared_from_this<����>��
Ȼ������ĳ�Ա�����п��Ե��� shared_from_this() ��������ȡһ��ָ��ǰ����� std::shared_ptr��
*/
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context& ioc, Server* server): _socket(ioc), _server(server) {
        // ÿ��session��һ��Ψһid
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }

    ~Session()
    {
        std::cout << "session destruct delete this: " << this << std::endl;
    }

    tcp::socket& Socket() { return _socket; }

    // ��start����������Կͻ��˵Ķ���д
    void Start();

    std::string& GetUuid();

private:
    // echo���͵ķ�����
    // ����Ļص�����һ������ָ����� �ӳ����ڴ����������
    void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred, std::shared_ptr<Session> _self_shared);
    void handle_write(const boost::system::error_code& error, std::shared_ptr<Session> _self_shared);

private:
    tcp::socket _socket;
    enum {max_length = 1024};
    char _data[max_length];
    Server* _server;
    std::string _uuid;
};

