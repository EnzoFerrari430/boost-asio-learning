#pragma once

#include <iostream>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
using boost::asio::ip::tcp;

class Server;

/*
std::enable_shared_from_this 是 C++ 标准库中的一个模板类，它在 <memory> 头文件中定义。
其主要作用是让一个对象能够安全地从其内部获取一个指向自身的 std::shared_ptr。

在使用 std::shared_ptr 管理对象生命周期时，有时对象自身需要创建一个指向自己的 std::shared_ptr.
但直接使用 this 指针创建 std::shared_ptr 会引发问题，
因为这样会产生多个独立管理同一对象的 std::shared_ptr，从而可能导致对象被多次删除，产生未定义行为。

要使用 std::enable_shared_from_this，需要让类继承自 std::enable_shared_from_this<类名>，
然后在类的成员函数中可以调用 shared_from_this() 方法来获取一个指向当前对象的 std::shared_ptr。
*/
class Session : public std::enable_shared_from_this<Session>
{
public:
    Session(boost::asio::io_context& ioc, Server* server): _socket(ioc), _server(server) {
        // 每个session有一个唯一id
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }

    ~Session()
    {
        std::cout << "session destruct delete this: " << this << std::endl;
    }

    tcp::socket& Socket() { return _socket; }

    // 在start函数里监听对客户端的读和写
    void Start();

    std::string& GetUuid();

private:
    // echo类型的服务器
    // 这里的回调增加一个智能指针参数 延长了内存的声明周期
    void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred, std::shared_ptr<Session> _self_shared);
    void handle_write(const boost::system::error_code& error, std::shared_ptr<Session> _self_shared);

private:
    tcp::socket _socket;
    enum {max_length = 1024};
    char _data[max_length];
    Server* _server;
    std::string _uuid;
};

