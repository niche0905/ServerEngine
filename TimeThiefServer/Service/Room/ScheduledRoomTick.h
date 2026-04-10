#pragma once

struct ScheduledRoomTick
{
    TimePoint       executeAt{};
    RoomId          roomId = 0;
};

struct ScheduledRoomTickCompare
{
    bool operator()(const ScheduledRoomTick& a, const ScheduledRoomTick& b) const
    {
        return a.executeAt > b.executeAt;  // executeAt이 작은 것이 우선순위가 높음
    }
};
