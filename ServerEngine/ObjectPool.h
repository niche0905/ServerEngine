#pragma once
#include "PoolObject.h"
#include <vector>
#include <queue>
#include <memory>
#include <mutex>

template <typename T>
requires PoolObject<T>
class ObjectPool
{
public:
    explicit ObjectPool(size_t initialSize = 10, bool threadSafe = false)
        : threadSafe_(threadSafe)
    {
        for (size_t i = 0; i < initialSize; ++i)
            pool_.push(std::make_unique<T>());
    }

    // 객체 얻기
    std::unique_ptr<T> Acquire()
    {
        if (threadSafe_) mutex_.lock();

        if (pool_.empty())
        {
            if (threadSafe_) mutex_.unlock();
            return std::make_unique<T>();   // 자동 확장
        }

        auto obj = std::move(pool_.front());
        pool_.pop();

        if (threadSafe_) mutex_.unlock();
        return obj;
    }

    // 객체 반환
    void Release(std::unique_ptr<T> obj)
    {
        if (threadSafe_) mutex_.lock();
        pool_.push(std::move(obj));
        if (threadSafe_) mutex_.unlock();
    }

    size_t Size() const
    {
        return pool_.size();
    }

private:
    std::queue<std::unique_ptr<T>> pool_;

    bool threadSafe_ = false;
    mutable std::mutex mutex_;

};
