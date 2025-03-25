
#include <iostream>
#include <boost/asio.hpp>
#include <set>
#include <memory>
using boost::asio::ip::tcp;

const int max_length = 1024;

typedef std::shared_ptr<tcp::socket> socket_ptr;
std::set<std::shared_ptr<std::thread>> thread_set;


// 服务器处理客户端的读写
void session(socket_ptr sock)
{
    try {
        for (;;)
        {
            char data[max_length];
            memset(data, 0, max_length);
            boost::system::error_code error;

            // 这个read它会读1024个字节， 所以如果客户端发送的数据量比较小的话 服务器会在这里进行等待 浪费资源
            //size_t length = boost::asio::read(sock, boost::asio::buffer(data, max_length), error);

            size_t length = sock->read_some(boost::asio::buffer(data, max_length), error);
            if (error == boost::asio::error::eof)
            {
                std::cout << "connection closed by peer" << std::endl;
                break;
            }
            else if (error)
            {
                throw boost::system::system_error(error);
            }

            std::cout << "receive from" << sock->remote_endpoint().address().to_string() << std::endl;
            std::cout << "receive message is " << data << std::endl;

            // 收到的数据回传给客户端
            boost::asio::write(*sock, boost::asio::buffer(data, length));

        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in thread: " << e.what() << "\n" << std::endl;
    }
}

/*
1 同步读写的缺陷在于读写是阻塞的，如果客户端对端不发送数据服务器的read操作是阻塞的，这将导致服务器处于阻塞等待状态。
2 可以通过开辟新的线程为新生成的连接处理读写，但是一个进程开辟的线程是有限的，约为2048个线程，在Linux环境可以通过unlimit增加一个进程开辟的线程数，但是线程过多也会导致切换消耗的时间片较多。
3 该服务器和客户端为应答式，实际场景为全双工通信模式，发送和接收要独立分开。
4 该服务器和客户端未考虑粘包处理。
综上所述，是我们这个服务器和客户端存在的问题，为解决上述问题，我们在接下里的文章里做不断完善和改进，主要以异步读写改进上述方案。
当然同步读写的方式也有其优点，比如客户端连接数不多，而且服务器并发性不高的场景，可以使用同步读写的方式。使用同步读写能简化编码难度。
*/ 
void server(boost::asio::io_context& io_context, unsigned short port)
{
    tcp::acceptor a(io_context, tcp::endpoint(tcp::v4(), port));
    for (;;)
    {
        socket_ptr socket(new tcp::socket(io_context));
        a.accept(*socket); //处理与客户端通信的socket
        // 创建一个线程去处理与客户端的读写
        auto t = std::make_shared<std::thread>(session, socket);
        // 放到集合里防止
        thread_set.insert(t);
    }
}

int main()
{

    // 子线程退出之后再退出主线程
    try {
        boost::asio::io_context ioc;
        server(ioc, 10086);
        for (auto& t : thread_set)
        {
            t->join();
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception in thread: " << e.what() << "\n" << std::endl;
    }
    return 0;
}