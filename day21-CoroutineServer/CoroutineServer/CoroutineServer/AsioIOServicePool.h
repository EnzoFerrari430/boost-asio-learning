#pragma once

#include <vector>
#include <memory>
#include <thread>

#include <boost/asio.hpp>

class AsioIOServicePool
{
public:
    using IOService = boost::asio::io_context;
    // boost 1.87中 boost::asio::io_context::work 类已经被弃用，并且被新的 boost::asio::executor_work_guard 
    using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;

    ~AsioIOServicePool();
    AsioIOServicePool(const AsioIOServicePool&) = delete;
    AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;

    boost::asio::io_context& GetIOService();
    void Stop();
    static AsioIOServicePool& GetInstance();
private:
    AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());

private:
    std::vector<IOService> _ioServices;
    std::vector<WorkPtr> _works;
    std::vector<std::thread> _threads;
    std::size_t _nextIOService;
};

