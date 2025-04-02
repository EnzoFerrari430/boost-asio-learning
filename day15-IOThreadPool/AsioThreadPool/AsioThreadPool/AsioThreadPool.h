#pragma once
#include "Singleton.h"
#include <boost/asio.hpp>
#include <memory>

class AsioThreadPool : public Singleton<AsioThreadPool>
{
    friend class Singleton<AsioThreadPool>;
public:
    using Work = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
    using WorkPtr = std::unique_ptr<Work>;

    ~AsioThreadPool() {}
    AsioThreadPool(const AsioThreadPool&) = delete;
    AsioThreadPool& operator=(const AsioThreadPool&) = delete;

    boost::asio::io_context& GetIOService();
    void Stop();
private:
    AsioThreadPool(int threadNum = std::thread::hardware_concurrency());

private:
    boost::asio::io_context _service;
    WorkPtr _work;
    std::vector<std::thread> _threads;
};

