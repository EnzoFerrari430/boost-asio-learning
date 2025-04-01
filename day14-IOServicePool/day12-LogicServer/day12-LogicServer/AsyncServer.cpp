#include <iostream>
#include <boost/asio.hpp>

#include "CServer.h"
#include "CSession.h"
#include "AsioIOServicePool.h"


/*
服务器的acceptor在主线程的ioc中接收连接
每个新的session关联AsioIOServicePool中的一个ioc,在这个ioc中轮询读写操作
*/

int main()
{
    try {
        auto pool = AsioIOServicePool::GetInstance();
        std::cout << "main thread is: " << std::this_thread::get_id() << std::endl;
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, pool](auto, auto) {
            ioc.stop();
            pool->Stop(); // 线程池优雅退出
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