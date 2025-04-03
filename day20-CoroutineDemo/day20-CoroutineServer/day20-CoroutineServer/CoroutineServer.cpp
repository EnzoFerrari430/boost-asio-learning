

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/write.hpp>
#include <iostream>

using boost::asio::ip::tcp;
using boost::asio::awaitable;
using boost::asio::co_spawn;
using boost::asio::detached;
using boost::asio::use_awaitable;

namespace this_coro = boost::asio::this_coro; // 当前协程执行的环境

awaitable<void> echo(tcp::socket socket)
{
    try {
        char data[1024];
        for (;;)
        {
            // 协程的方式读取数据
            std::size_t n = co_await socket.async_read_some(boost::asio::buffer(data, 1024), use_awaitable);
            std::cout << "received msg: " << std::string(data, n) << '\n';
            // 协程的方式发送数据
            co_await async_write(socket, boost::asio::buffer(data, n), use_awaitable);
        }
    }
    catch (std::exception& e)
    {
        std::cout << "Exception is: " << e.what() << '\n';
    }
}

awaitable<void> listener()
{
    auto executor = co_await this_coro::executor;
    tcp::acceptor acceptor(executor, { tcp::v4(), 10086 });
    for (;;)
    {
        // 协程的方式获取连接
        tcp::socket socket = co_await acceptor.async_accept(use_awaitable);
        co_spawn(executor, echo(std::move(socket)), detached);
    }
}

int main()
{

    try {
        boost::asio::io_context io_context(1);
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        signals.async_wait([&](auto, auto) {
            io_context.stop();
        });

        // 启动协程
        co_spawn(io_context, listener(), detached);

        io_context.run();
    }
    catch (std::exception& e)
    {
        std::cout << "Exception is: " << e.what() << '\n';
    }

    return 0;
}