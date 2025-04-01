#include "AsioIOServicePool.h"


AsioIOServicePool::AsioIOServicePool(std::size_t size)
    : _ioServices(size), _works(size), _nextIOService(0)
{
    /*
    executor_work_guard 的核心功能是阻止与之关联的执行器（如 io_context 的执行器）在还有未完成的异步操作时退出。
    在 Boost.Asio 中，执行器负责调度和执行异步任务，当调用执行器的 run() 方法时，它会不断地从任务队列中取出任务并执行，
    直到任务队列为空且没有更多的 “工作”。executor_work_guard 通过向执行器表明存在 “工作”，
    使得执行器即使在任务队列为空时也会保持运行状态，等待后续可能到来的异步操作。
    */
    for (std::size_t i = 0; i < size; ++i)
    {
        _works[i] = std::make_unique<Work>(boost::asio::make_work_guard(_ioServices[i].get_executor()));
    }

    // 遍历多个ioservice 创建多个线程 每个线程内部启动ioservice
    for (std::size_t i = 0; i < size; ++i)
    {
        _threads.emplace_back([this, i]() {
            _ioServices[i].run();
        });
    }
}

AsioIOServicePool::~AsioIOServicePool()
{
    std::cout << "AsioIOServicePool destruct" << std::endl;
}

boost::asio::io_context& AsioIOServicePool::GetIOService()
{
    auto& service = _ioServices[_nextIOService++];
    if (_nextIOService == _ioServices.size())
        _nextIOService = 0;
    return service;
}

void AsioIOServicePool::Stop()
{
    for (auto& work : _works)
    {
        // executor_work_guard 对象被销毁，关联的io_context停止运行。
        work.reset();
    }

    // 阻塞在这里
    // io_context所在的线程退出
    for (auto& t : _threads)
    {
        t.join();
    }
}
