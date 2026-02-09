#pragma once
#include "ReplicationTypes.h"
#include <vector>

enum class RepEventType : uint8
{
    None = 0,
    
    Hit,
    Death,
    Fire,
    Interact,
    
    // ... 확장 가능
};

struct RepEvent
{
    RepEventType type{RepEventType::None};
    RepObjectId source{};
    RepObjectId target{};
    
    // 추가 데이터
    int32 a{0};
    int32 b{0};
    
    uint64 tMs;
};

class ReplicationEventQueue
{
public:
    void Push(const RepEvent& event) { events_.push_back(event); }
    void Clear() { events_.clear(); }
    bool Empty() const { return events_.empty(); }
    
    const std::vector<RepEvent>& Events() const { return events_; }
    
private:
    std::vector<RepEvent> events_;
    
};
