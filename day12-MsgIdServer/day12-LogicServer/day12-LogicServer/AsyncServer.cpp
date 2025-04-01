#include <iostream>
#include <boost/asio.hpp>

#include "CServer.h"
#include "CSession.h"

int main()
{
    try {
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](auto, auto) {
            ioc.stop();
        });

        CServer s(ioc, 10086);
        ioc.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}