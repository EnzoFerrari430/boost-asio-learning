#pragma once

#include <memory>

#include <boost/asio.hpp>

#include "request_handler.hpp"
#include "request.hpp"
#include "request_parser.hpp"
#include "reply.hpp"

namespace http {
    namespace server {

        class connection_manager;

        class connection : public std::enable_shared_from_this<connection>
        {
        public:
            connection(const connection&) = delete;
            connection& operator=(const connection&) = delete;

            explicit connection(boost::asio::ip::tcp::socket socket, connection_manager& manager, request_handler& handler);

            void start();
            void stop();
        private:
            void do_read();
            void do_write();

        private:
            boost::asio::ip::tcp::socket _socket;
            connection_manager& _connection_manager;

            /// The handler used to process the incoming request.
            request_handler& _request_handler;

            /// Buffer for incoming data.
            std::array<char, 8192> _buffer;

            /// The incoming request.
            request _request;


            request_parser _request_parser;
            reply _reply;

        };

        typedef std::shared_ptr<connection> connection_ptr;

    }
}

