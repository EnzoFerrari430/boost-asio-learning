

#include "Client.h"


int main(int argc, char* argv[])
{
    try
    {
        /*   if (argc != 3)
           {
               std::cout << "Usage: async_client <server> <path>\n";
               std::cout << "Example:\n";
               std::cout << "  async_client www.boost.org /LICENSE_1_0.txt\n";
               return 1;
           }*/

        boost::asio::io_context io_context;
        Client c(io_context, "127.0.0.1:8080", "/");
        io_context.run();
        getchar();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception: " << e.what() << "\n";
    }

    return 0;
}