#pragma once

/*
单例类
*/

#include <memory>
#include <mutex>
#include <iostream>

template<typename T>
class Singleton
{
public:
    static std::shared_ptr<T> GetInstance()
    {
        static std::once_flag s_flag;
        std::call_once(s_flag, [&]() {
            _instance = std::shared_ptr<T>(new T);
        });

        return _instance;
    }

    void PrintAddress()
    {
        std::cout << _instance->get() << std::endl;
    }

protected:
    Singleton() = default;
    ~Singleton()
    {
        std::cout << "this is singleton destruct" << std::endl;
    }
private:
    Singleton(const Singleton<T>&) = delete;
    Singleton& operator=(const Singleton<T>& st) = delete;

    static std::shared_ptr<T> _instance;
};

template<typename T>
std::shared_ptr<T> Singleton<T>::_instance = nullptr;
