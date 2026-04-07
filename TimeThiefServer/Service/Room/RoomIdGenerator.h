#pragma once
#include <atomic>

class RoomIdGenerator
{
public:
    RoomIdGenerator() = default;
    
    RoomId Generate()
    {
        return nextId_.fetch_add(1, std::memory_order_relaxed);
    }
    
private:
    std::atomic<RoomId> nextId_{ 1 };
    
};