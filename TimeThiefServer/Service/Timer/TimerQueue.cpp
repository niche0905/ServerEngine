#include "pch.h"
#include "TimerQueue.h"
#include "TimerTask.h"

/*--------------
   TimerQueue
--------------*/

TimerId TimerQueue::ScheduleAt(TimePoint executeAt, Job callback)
{
   if (!callback)
      return 0;
   
   TimerId timerId = nextTimerId_++;
   heap_.push(TimerTask{ executeAt, timerId, std::move(callback) });
   
   return timerId;
}

TimerId TimerQueue::ScheduleAfter(Duration delay, Job callback)
{
   if (!callback)
      return 0;
   
   return ScheduleAt(Clock::now() + delay, std::move(callback));
}

bool TimerQueue::Cancel(TimerId timerId)
// lazy cancel 이므로 이미 실행된 Timer의 경우도 true로 반환될 수 있음
{
   if (timerId == 0)
      return false;
   
   cancelled_.insert(timerId);
   return true;
}

void TimerQueue::PopExpired(TimePoint now, std::vector<Job>& outJobs)
{
   outJobs.clear();
   
   while (not heap_.empty()) {
      const TimerTask& top = heap_.top();
      if (top.executeAt > now)
         break;
      
      TimerTask task = top;   // 복사 허용..
      heap_.pop();
      
      if (cancelled_.erase(task.timerId) > 0) {
         continue;
      }
      
      if (task.callback) {
         outJobs.push_back(std::move(task.callback));
      }
   }
}

bool TimerQueue::Empty() const
{
   return heap_.empty();
}

size_t TimerQueue::Size() const
{
   return heap_.size();
}

std::optional<TimePoint> TimerQueue::GetNextExecuteAt() const
{
   if (heap_.empty())
      return std::nullopt;
   
   return heap_.top().executeAt;
}

void TimerQueue::Clear()
{
   while (!heap_.empty()) {
      heap_.pop();
   }
   
   cancelled_.clear();
}
