#include <iostream>
#include <boost/asio.hpp>

#include "CServer.h"
#include "CSession.h"

// 服务器优雅退出
#include <csignal>
#include <thread>
#include <mutex>

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

// 处理退出信号
void sig_handler(int sig)
{
    if (sig == SIGINT || sig == SIGTERM)
    {
        std::unique_lock<std::mutex> lock_quit(mutex_quit);
        bstop = true;
        cond_quit.notify_one();
    }
}

int main()
{
    try {
        boost::asio::io_context ioc;
        std::thread net_work_thread([&ioc]() {
            CServer s(ioc, 10086);
            ioc.run(); // 在网络线程中启动io_context的事件循环
        });

        // 注册信号，绑定处理函数
        signal(SIGINT, sig_handler);
        signal(SIGTERM, sig_handler);

        while (!bstop)
        {
            std::unique_lock<std::mutex> lock_quit(mutex_quit);
            cond_quit.wait(lock_quit);
        }
        ioc.stop();
        net_work_thread.join();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}