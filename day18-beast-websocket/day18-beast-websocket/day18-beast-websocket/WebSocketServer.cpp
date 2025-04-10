#include "WebSocketServer.h"
#include <iostream>

WebSocketServer::WebSocketServer(net::io_context& ioc, unsigned short port)
    : _ioc(ioc), _acceptor(ioc, net::ip::tcp::endpoint(net::ip::tcp::v4(), port))
{
    std::cout << "Server start on port: " << port << std::endl;
}

void WebSocketServer::StartAccept()
{
    auto con_ptr = std::make_shared<Connection>(_ioc);
    _acceptor.async_accept(con_ptr->GetSocket(), [this, con_ptr](boost::system::error_code err) {
        try {
            if (!err)
            {
                con_ptr->AsyncAccept();
            }
            else
            {
                std::cout << "acceptor async_accept failed, err is " << err.what() << std::endl;
            }

            StartAccept();
        }
        catch (std::exception& exp)
        {
            std::cout << "async accept exception is " << exp.what() << std::endl;
        }
    });
}
