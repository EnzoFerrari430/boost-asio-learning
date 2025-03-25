#include <iostream>
#include <boost/asio.hpp>

#include "Server.h"
#include "Session.h"

/*

本文介绍了异步的应答服务器设计，但是这种服务器并不会在实际生产中使用，主要有两个原因:
1   因为该服务器的发送和接收以应答的方式交互，而并不能做到应用层想随意发送的目的，也就是未做到完全的收发分离(全双工逻辑)。
2   该服务器未处理粘包，序列化，以及逻辑和收发线程解耦等问题。
3   该服务器存在二次析构的风险。
这些问题我们会在接下来的文章中不断完善
*/

int main()
{
    try {
        boost::asio::io_context ioc;

        Server s(ioc, 10086);
        ioc.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}