
#include <iostream>
#include <boost/asio.hpp>

using namespace std;
using namespace boost::asio::ip;

const int MAX_LENGTH = 1024;

int main()
{

    try {
        boost::asio::io_context ioc;
        boost::asio::ip::tcp::endpoint remote_ep(make_address("127.0.0.1"), 10086);
        tcp::socket sock(ioc);
        boost::system::error_code error = boost::asio::error::host_not_found;
        sock.connect(remote_ep, error);
        if (error)
        {
            std::cout << "connect failed, code is " << error.value() << " error msg is " << error.message() << '\n';
        }

        std::cout << "Enter message: ";
        char request[MAX_LENGTH];
        std::cin.getline(request, MAX_LENGTH);
        size_t request_length = strlen(request);
        boost::asio::write(sock, boost::asio::buffer(request, request_length));

        char reply[MAX_LENGTH];
        size_t reply_length = boost::asio::read(sock, boost::asio::buffer(reply, request_length));

        std::cout << "reply is " << std::string(reply, reply_length) << '\n';
        getchar();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception is " << e.what() << std::endl;
    }

    return 0;

}