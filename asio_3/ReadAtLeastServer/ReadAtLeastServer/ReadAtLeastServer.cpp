#include <iostream>

#include <boost/asio.hpp>
#include "CServer.h"

int main()
{

    try {
        boost::asio::io_context ioc;
        CServer server(ioc, 10086);

        ioc.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}