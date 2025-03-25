#include "endpoint.h"

#include "boost/asio.hpp"
#include <iostream>

using namespace boost;

int client_end_point()
{
    std::string raw_ip_address = "127.4.8.1";
    unsigned short port_num = 3333;


    // 1.获取IP地址
    boost::system::error_code ec;
    // 1.87没有这个函数 使用make_address代替
    //asio::ip::address ip_address = asio::ip::address::from_string()

    asio::ip::address ip_address = asio::ip::make_address(raw_ip_address, ec);
    if (ec.value() != 0)
    {
        std::cout << "Failed to parse the IP address. Error code= " << ec.value() << " .Message is " << ec.message() << std::endl;
        return ec.value();
    }

    // 2.绑定端口号
    asio::ip::tcp::endpoint ep(ip_address, port_num);
    return 0;
}

int server_end_point()
{
    unsigned short port_num = 3333;
    asio::ip::address ip_address = asio::ip::address_v6::any();

    asio::ip::tcp::endpoint ep(ip_address, port_num);
    return 0;
}

// 客户端生成socket
int create_tcp_socket()
{
    //asio网络通信核心上下文组件io_context
    asio::io_context ioc;
    asio::ip::tcp protocol = asio::ip::tcp::v4();
    // 创建一个socket
    asio::ip::tcp::socket sock(ioc);
    boost::system::error_code ec;
    sock.open(protocol, ec);
    if (ec.value() != 0)
    {
        std::cout << "Failed to open the socket! Error code = " << ec.value() << " .Message: " << ec.message() << std::endl;
        return ec.value();
    }

    return 0;

}

int create_acceptor_socket()
{
    asio::io_context ios;
    // 老写法
    //asio::ip::tcp::acceptor acceptor(ios);
    //asio::ip::tcp protocol = asio::ip::tcp::v4();
    //boost::system::error_code ec;
    //acceptor.open(protocol, ec);
    //if (ec.value() != 0)
    //{
    //    std::cout << "Failed to open the acceptor socket! Error code = " << ec.value() << " .Message: " << ec.message() << std::endl;
    //    return ec.value();
    //}

    // 新写法
    asio::ip::tcp::acceptor a(ios, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 3333));
}

int bind_acceptor_socket()
{
    unsigned short port_num = 3333;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
    asio::io_context ios;
    asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
    boost::system::error_code ec;
    acceptor.bind(ep, ec);
    if (ec.value() != 0)
    {
        // Failed to bidn the acceptor socket. breaking
        std::cout << "Failed to bind the acceptor socket. "
            << "Error code= " << ec.value() << " .Message: " << ec.message() << std::endl;
        return ec.value();
    }

    return 0;

}

int connect_to_end()
{
    std::string raw_ip_address = "192.168.1.124";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ios;
        asio::ip::tcp::socket sock(ios, ep.protocol());
        sock.connect(ep);
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code() << " .Message: " << e.what() << std::endl;
        return e.code().value();
    }
    return 0;
}

int dns_connect_to_end()
{
    std::string host = "llfc.club";
    std::string port_num = "3333";
    asio::io_context ios;

    // dns解析器
    asio::ip::basic_resolver_query<asio::ip::tcp> query(host, port_num, asio::ip::resolver_query_base::flags::numeric_service);
    asio::ip::tcp::resolver resolver(ios);
    try {
        asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port_num, asio::ip::resolver_query_base::flags::numeric_service);
        asio::ip::tcp::socket sock(ios);
        asio::connect(sock, endpoints);
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code() << " .Message: " << e.what() << std::endl;
        return e.code().value();
    }
    return 0;
}

int accept_new_connection()
{
    // 链接来不及处理的链接数缓冲区大小
    const int BACKLOG_SIZE = 30;

    unsigned short port_num = 3333;
    asio::ip::tcp::endpoint ep(asio::ip::address_v4::any(), port_num);
    asio::io_context ios;
    try {
        asio::ip::tcp::acceptor acceptor(ios, ep.protocol());
        acceptor.bind(ep);
        acceptor.listen(BACKLOG_SIZE);
        // 客户端新的链接sock
        asio::ip::tcp::socket sock(ios);
        acceptor.accept(sock);
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code() << " .Message: " << e.what() << std::endl;
        return e.code().value();
    }
    return 0;
}

// 用于发送的buffer
void use_const_buffer()
{
    std::string buf = "hello world";
    asio::const_buffer asio_buf(buf.c_str(), buf.length());

    // 发送的数据需要一个ConstBufferSequence的模板类型
    // 所以这里需要一个vector
    std::vector<asio::const_buffer> buffers_sequence;
    buffers_sequence.push_back(asio_buf);

    //asio::write()
}

// 字符串
void use_buffer_str()
{
    asio::const_buffer output_buf = asio::buffer("hello world");
}

// 数组
void use_buffer_array()
{
    const size_t BUF_SIZE_BYTES = 20;
    std::unique_ptr<char[]> buf(new char[BUF_SIZE_BYTES]);
    asio::const_buffer input_buf = asio::buffer(static_cast<void*>(buf.get()), BUF_SIZE_BYTES);
}

void write_to_socket(asio::ip::tcp::socket& sock)
{
    std::string buf = "Hello World!";
    std::size_t total_bytes_written = 0;
    // 循环发送
    // write_some 返回每次写入的字节数
    while (total_bytes_written != buf.length())
    {
        total_bytes_written += sock.write_some(asio::buffer(buf.c_str() + total_bytes_written,
            buf.length() - total_bytes_written));
    }
}

// 发送数据的代码片段
// 同步发送
int send_data_by_write_some()
{
    std::string raw_ip_address = "192.168.3.11";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);
        write_to_socket(sock);
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code() << " .Message: " << e.what() << std::endl;
        return e.code().value();
    }
    return 0;
}

// 同步阻塞发送, 要么全部发送完 要么出现错误
// send_length 有三种情况 <0 表示系统错误  =0 对端关闭 =buf.length() 全部发送完
int send_data_by_send()
{
    std::string raw_ip_address = "192.168.3.11";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);

        std::string buf = "Hello World!";
        int send_length = sock.send(asio::buffer(buf.c_str(), buf.length()));
        if (send_length <= 0)
            return 0;
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code() << " .Message: " << e.what() << std::endl;
        return e.code().value();
    }
    return 0;
}

int send_data_by_write()
{
    std::string raw_ip_address = "192.168.3.11";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);

        std::string buf = "Hello World!";

        int send_length = asio::write(sock, asio::buffer(buf.c_str(), buf.length()));
        if (send_length <= 0)
            return 0;
    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code() << " .Message: " << e.what() << std::endl;
        return e.code().value();
    }
    return 0;
}

// 读数据
std::string read_from_socket(asio::ip::tcp::socket& sock)
{
    const unsigned char MESSAGE_SIZE = 7;
    char buf[MESSAGE_SIZE];
    std::size_t total_bytes_read = 0;
    while (total_bytes_read != MESSAGE_SIZE)
    {
        total_bytes_read += sock.read_some(asio::buffer(buf + total_bytes_read, MESSAGE_SIZE - total_bytes_read));
    }

    return std::string(buf, total_bytes_read);
}

// 读数据的全流程
int read_data_by_read_some()
{
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);
        read_from_socket(sock);

    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code() << " .Message: " << e.what() << std::endl;
        return e.code().value();
    }
}

int read_data_by_receive()
{
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);

        // 使用socket类的API
        const unsigned char BUFF_SIZE = 7;
        char buffer_receive[BUFF_SIZE];
        int receive_length = sock.receive(asio::buffer(buffer_receive, BUFF_SIZE));
        if (receive_length <= 0)
        {
            std::cout << "receive failed " << std::endl;
        }

    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code() << " .Message: " << e.what() << std::endl;
        return e.code().value();
    }
}


int read_data_by_read()
{
    std::string raw_ip_address = "127.0.0.1";
    unsigned short port_num = 3333;
    try {
        asio::ip::tcp::endpoint ep(asio::ip::make_address(raw_ip_address), port_num);
        asio::io_context ioc;
        asio::ip::tcp::socket sock(ioc, ep.protocol());
        sock.connect(ep);

        // 使用asio全局API read
        const unsigned char BUFF_SIZE = 7;
        char buffer_receive[BUFF_SIZE];
        int receive_length = asio::read(sock, asio::buffer(buffer_receive, BUFF_SIZE));
        if (receive_length <= 0)
        {
            std::cout << "receive failed " << std::endl;
        }

    }
    catch (system::system_error& e)
    {
        std::cout << "Error occured! Error code = " << e.code() << " .Message: " << e.what() << std::endl;
        return e.code().value();
    }
}
