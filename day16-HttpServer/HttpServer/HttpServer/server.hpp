#pragma once

#include <boost/asio.hpp>

#include "connection.hpp"
#include "connection_manager.hpp"
#include "request_handler.hpp"


namespace http {
    namespace server {

        class server
        {
        public:
            server(const server&) = delete;
            server& operator=(const server&) = delete;

            explicit server(const std::string& address, const std::string& port, const std::string& doc_root);

            void run();

        private:
            void do_accept();

            void do_await_stop();
        private:
            boost::asio::io_context _io_context;
            boost::asio::ip::tcp::socket _socket;
            boost::asio::ip::tcp::acceptor _acceptor;
            boost::asio::signal_set _signals;


            //
            connection_manager _connection_manager;

            /// The handler for all incoming requests.
            request_handler _request_handler;
        };

    }
}

