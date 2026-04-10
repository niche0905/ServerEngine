#include "pch.h"
#include "RoomTickScheduler.h"

/*---------------------
   RoomTickScheduler
---------------------*/

void RoomTickScheduler::Schedule(RoomId roomId, TimePoint executeAt)
{
   if (roomId == 0) return;   // 유효하지 않은 roomId는 스케줄링하지 않음
   
   queue_.push(ScheduledRoomTick{executeAt, roomId});
}

bool RoomTickScheduler::HasDueTick(TimePoint now) const
{
   if (queue_.empty()) return false;
   
   return queue_.top().executeAt <= now;
}

bool RoomTickScheduler::PopDue(TimePoint now, ScheduledRoomTick& outTick)
{
   if (!HasDueTick(now)) return false;
   
   outTick = queue_.top();
   queue_.pop();
   return true;
}

bool RoomTickScheduler::Empty() const
{
   return queue_.empty();
}

size_t RoomTickScheduler::Size() const
{
   return queue_.size();
}

void RoomTickScheduler::Clear()
{
   queue_ = Queue{};
}
