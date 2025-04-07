#include "connection.hpp"
#include "connection_manager.hpp"

namespace http {
    namespace server {

        connection::connection(boost::asio::ip::tcp::socket socket, connection_manager& manager, request_handler& handler)
            : _socket(std::move(socket)), _connection_manager(manager), _request_handler(handler)
        {

        }

        void connection::start()
        {
            do_read();
        }

        void connection::stop()
        {
            _socket.close();
        }

        void connection::do_read()
        {
            auto self(shared_from_this());
            _socket.async_read_some(boost::asio::buffer(_buffer), [this, self](boost::system::error_code ec, std::size_t bytes_transferred) {
                if (!ec)
                {
                    request_parser::result_type result;
                    std::tie(result, std::ignore) = _request_parser.parse(_request, _buffer.data(), _buffer.data() + bytes_transferred);

                    if (result == request_parser::good)
                    {
                        _request_handler.handle_request(_request, _reply);
                        do_write();
                    }
                    else if (result == request_parser::bad)
                    {
                        _reply = reply::stock_reply(reply::bad_request);
                        do_write();
                    }
                    else
                    {
                        do_read();
                    }
                }
                else if(ec != boost::asio::error::operation_aborted)
                {
                    _connection_manager.stop(shared_from_this());
                }
            });
        }

        void connection::do_write()
        {
            auto self(shared_from_this());
            boost::asio::async_write(_socket, _reply.to_buffers(), [this, self](boost::system::error_code ec, std::size_t n) {

                if (!ec)
                {
                    // Initiate graceful connection closure.
                    boost::system::error_code ignored_ec;
                    _socket.shutdown(boost::asio::ip::tcp::socket::shutdown_both,
                        ignored_ec);
                }
                if (ec != boost::asio::error::operation_aborted)
                {
                    _connection_manager.stop(shared_from_this());
                }

            });
        }

    }
}
