#pragma once
#include <queue>
#include <vector>
#include <chrono>
#include "ScheduledRoomTick.h"

/*---------------------
   RoomTickScheduler
---------------------*/
//
// RoomTickScheduler는 Shard에서 Room의 Tick을 관리하는 클래스입니다.
//

class RoomTickScheduler
{
public:
   using Queue = std::priority_queue<ScheduledRoomTick, std::vector<ScheduledRoomTick>, ScheduledRoomTickCompare>;
   
public:
   RoomTickScheduler() = default;
   ~RoomTickScheduler() = default;
   
   RoomTickScheduler(const RoomTickScheduler&) = delete;
   RoomTickScheduler& operator=(const RoomTickScheduler&) = delete;
   
public:
   void Schedule(RoomId roomId, TimePoint executeAt);
   
   bool HasDueTick(TimePoint now) const;
   bool PopDue(TimePoint now, ScheduledRoomTick& outTick);
   
   bool Empty() const;
   size_t Size() const;
   void Clear();
   
private:
   Queue queue_;
    
};
