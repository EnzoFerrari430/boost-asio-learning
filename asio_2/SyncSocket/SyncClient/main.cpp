#include <iostream>

#include <boost/asio.hpp>

using namespace boost::asio::ip;

// 最大接收长度
const int MAX_LENGTH = 1024;

int main()
{
    try {
        // 1.创建上下文服务
        boost::asio::io_context ioc;
        // 对端地址
        tcp::endpoint remote_ep(make_address("127.0.0.1"), 10086);
        tcp::socket sock(ioc, remote_ep.protocol());
        boost::system::error_code error = boost::asio::error::host_not_found;
        sock.connect(remote_ep, error);
        if (error)
        {
            std::cout << "connect failed, code is " << error.value() << " error msg is " << error.message() << std::endl;
        }

        std::cout << "Enter message: ";
        char request[MAX_LENGTH];
        std::cin.getline(request, MAX_LENGTH);
        size_t request_length = strlen(request);
        boost::asio::write(sock, boost::asio::buffer(request, request_length));

        char reply[MAX_LENGTH];
        size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply, request_length));
        std::cout << "Reply is: ";
        std::cout.write(reply, reply_length);
        std::cout << std::endl;
    }
    catch (boost::system::system_error& e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}