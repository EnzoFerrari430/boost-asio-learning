#pragma once

#include <vector>

#include <boost/asio.hpp>

#include "header.hpp"

namespace http {
    namespace server {

        struct reply
        {
            enum status_type
            {
                ok = 200,
                created = 201,
                accepted = 202,
                no_content = 204,
                multiple_choices = 300,
                moved_permanently = 301,
                moved_temporarily = 302,
                not_modified = 304,
                bad_request = 400,
                unauthorized = 401,
                forbidden = 403,
                not_found = 404,
                internal_server_error = 500,
                not_implemented = 501,
                bad_gateway = 502,
                service_unavailable = 503
            } status;

            std::vector<header> headers;

            std::string content;

            std::vector<boost::asio::const_buffer> to_buffers();

            static reply stock_reply(status_type status);
        };

    }
}