
#include <iostream>
#include <string>
#include <filesystem>

#include <boost/asio.hpp>

#include "server.hpp"

int main(int argc, char* argv[])
{

    try {

        std::filesystem::path path = std::filesystem::current_path() / "res";

        std::cout << "Path: " << path.string() << '\n';
        std::cout << "Usage: http_server<127.0.0.1> <8080> " << path.string() << "\n";

        http::server::server s("127.0.0.1", "8080", path.string());

        s.run();

    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << std::endl;
    }

    return 0;

}