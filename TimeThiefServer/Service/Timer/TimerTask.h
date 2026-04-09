#pragma once
#include "Utils/Types.h"

struct TimerTask
{
    TimePoint   executeAt;
    TimerId     timerId;
    Job         callback;
    
    bool operator>(const TimerTask& other) const
    {
        return executeAt > other.executeAt;
    }
};

struct TimerTaskCompare
{
    bool operator()(const TimerTask& a, const TimerTask& b) const
    {
        return a.executeAt > b.executeAt;
    }
};
