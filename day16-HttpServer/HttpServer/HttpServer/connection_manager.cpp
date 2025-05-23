#include "connection_manager.hpp"

namespace http {
    namespace server {

        connection_manager::connection_manager()
        {

        }

        void connection_manager::start(connection_ptr c)
        {
            _connections.insert(c);
            c->start();
        }

        void connection_manager::stop(connection_ptr c)
        {
            _connections.erase(c);
            c->stop();
        }

        void connection_manager::stop_all()
        {
            for (auto c : _connections)
            {
                c->stop();
            }
            _connections.clear();
        }

    }
}