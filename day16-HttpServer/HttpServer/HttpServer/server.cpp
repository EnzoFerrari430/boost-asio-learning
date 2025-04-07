#include "server.hpp"
#include <iostream>

namespace http {
    namespace server {

        server::server(const std::string& address, const std::string& port, const std::string& doc_root)
            : _io_context(), _socket(_io_context), _acceptor(_io_context), _signals(_io_context)
            , _request_handler(doc_root)
        {

            _signals.add(SIGINT);
            _signals.add(SIGTERM);
#if define(SIGQUIT)
            _signals.add(SIGQUIT);
#endif


            do_await_stop();

            //
            // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
            boost::asio::ip::tcp::resolver resolver(_io_context);
            boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(address, port);
            boost::asio::ip::tcp::resolver::results_type::iterator it = endpoints.begin();
            boost::asio::ip::tcp::endpoint endpoint;
            if (it != endpoints.end()) {
                endpoint = *it;
                std::cout << "First resolved endpoint: " << endpoint << std::endl;
            }
            else {
                std::cout << "No endpoints were resolved." << std::endl;
            }
            _acceptor.open(endpoint.protocol());
            _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
            _acceptor.bind(endpoint);
            _acceptor.listen();

            do_accept();

        }

        void server::run()
        {
            _io_context.run();
        }

        // 异步处理连接
        void server::do_accept()
        {
            _acceptor.async_accept(_socket, [this](const boost::system::error_code& ec) {
            
                if (!_acceptor.is_open())
                {
                    return;
                }

                if (!ec)
                {
                    _connection_manager.start(std::make_shared<connection>(std::move(_socket), _connection_manager, _request_handler));
                }

                do_accept();

            });
        }


        void server::do_await_stop()
        {
            _signals.async_wait([this](boost::system::error_code, int) {
                _acceptor.close();
                _connection_manager.stop_all();
            });
        }

    }
}

