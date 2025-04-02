#include <iostream>
#include <boost/asio.hpp>

#include "CServer.h"
#include "CSession.h"
#include "AsioThreadPool.h"

#include <mutex>
#include <condition_variable>

bool bstop = false;
std::condition_variable cond_quit;
std::mutex mutex_quit;

/*
初始化一个iocontext用来监听服务器的读写事件，包括新连接到来的监听也用这个iocontext。
多线程跑同一个iocontext.run() 监听服务器的读写的事件

在多线程中回调函数会被不同的线程触发

隐患
IOThreadPool模式有一个隐患，同一个socket的就绪后，触发的回调函数可能在不同的线程里，比如第一次是在线程1，第二次是在线程3，如果这两次触发间隔时间不大，那么很可能出现不同线程并发访问数据的情况，比如在处理读事件时，
第一次回调触发后我们从socket的接收缓冲区读数据出来，第二次回调触发,还是从socket的接收缓冲区读数据，就会造成两个线程同时从socket中读数据的情况，会造成数据混乱。

利用strand改进
对于多线程触发回调函数的情况，我们可以利用asio提供的串行类strand封装一下，这样就可以被串行调用了，其基本原理就是在线程各自调用函数时取消了直接调用的方式，
而是利用一个strand类型的对象将要调用的函数投递到strand管理的队列中，再由一个统一的线程调用回调函数，调用是串行的，解决了线程并发带来的安全问题。

*/

int main()
{
    try {
        auto pool = AsioThreadPool::GetInstance();
        std::cout << "main thread is: " << std::this_thread::get_id() << std::endl;
        boost::asio::io_context ioc;
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc, pool](auto, auto) {
            ioc.stop();
            pool->Stop(); // 线程池优雅退出
            std::unique_lock<std::mutex> lock(mutex_quit);
            bstop = true;
            cond_quit.notify_one();
        });

        CServer s(pool->GetIOService(), 10086);
        {
            std::unique_lock<std::mutex> lock(mutex_quit);
            while (!bstop)
            {
                cond_quit.wait(lock);
            }
        }
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}