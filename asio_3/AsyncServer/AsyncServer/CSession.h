#pragma once

#include <iostream>
#include <queue>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
using boost::asio::ip::tcp;

class CServer;
class MsgNode;

/*
std::enable_shared_from_this 是 C++ 标准库中的一个模板类，它在 <memory> 头文件中定义。
其主要作用是让一个对象能够安全地从其内部获取一个指向自身的 std::shared_ptr。

在使用 std::shared_ptr 管理对象生命周期时，有时对象自身需要创建一个指向自己的 std::shared_ptr.
但直接使用 this 指针创建 std::shared_ptr 会引发问题，
因为这样会产生多个独立管理同一对象的 std::shared_ptr，从而可能导致对象被多次删除，产生未定义行为。

要使用 std::enable_shared_from_this，需要让类继承自 std::enable_shared_from_this<类名>，
然后在类的成员函数中可以调用 shared_from_this() 方法来获取一个指向当前对象的 std::shared_ptr。
*/


class CSession : public std::enable_shared_from_this<CSession>
{
public:
    CSession(boost::asio::io_context& ioc, CServer* server): _socket(ioc), _server(server) {
        // 每个session有一个唯一id
        boost::uuids::uuid a_uuid = boost::uuids::random_generator()();
        _uuid = boost::uuids::to_string(a_uuid);
    }

    ~CSession()
    {
        std::cout << "session destruct delete this: " << this << std::endl;
    }

    tcp::socket& Socket() { return _socket; }

    // 在start函数里监听对客户端的读和写
    void Start();

    std::string& GetUuid();

    void Send(char* msg, int max_length);

private:
    // echo类型的服务器
    // 这里的回调增加一个智能指针参数 延长了内存的声明周期
    void HandleRead(const boost::system::error_code& error, std::size_t bytes_transferred, std::shared_ptr<CSession> _self_shared);
    void HandleWrite(const boost::system::error_code& error, std::shared_ptr<CSession> _self_shared);

private:
    tcp::socket _socket;
    enum {max_length = 1024};
    char _data[max_length];
    CServer* _server;
    std::string _uuid;


    /*
    因为全双工要有自己的发送接口，然而我们不能保证每次调用的发送接口的时候上一次的数据已经发送完，
    所以我们要把要发送的数据放入到队列中，通过回调不停的发送。
    而且我们不能保证发送的接口和回调的函数的接口在一个线程里，所以要增加一个锁保证队列的安全性 
    */
    std::queue<std::shared_ptr<MsgNode>> _send_que;
    std::mutex _send_lock;
};

