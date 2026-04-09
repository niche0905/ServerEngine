#pragma once

struct TimerTaskCompare;
struct TimerTask;

/*--------------
   TimerQueue
--------------*/
//
// TimerQueue는 타이머 이벤트를 관리하는 클래스입니다.
// Shard에 배치되어 Data Race가 없는 상황을 상정하고 만들어 졌습니다.
// 따라서 Shard가 아닌 다른 곳에서 TimerQueue에 접근할 때는 반드시 Shard의 Job Queue에 작업을 넣어서 접근해야 합니다.
//

class TimerQueue
{
public:
   TimerQueue() = default;
   
   TimerId ScheduleAt(TimePoint executeAt, Job callback);
   TimerId ScheduleAfter(Duration delay, Job callback);
   
   bool Cancel(TimerId timerId);
   
   void PopExpired(TimePoint now, std::vector<TimerTask>& outJobs);
   
   bool Empty() const;
   size_t Size() const;
   
   std::optional<TimePoint> GetNextExecuteAt() const;
   
   void Clear();
   
private:
   std::priority_queue<TimerTask, std::vector<TimerTask>, TimerTaskCompare> heap_;
   std::unordered_set<TimerId> cancelled_;
   TimerId nextTimerId_ = 1;
    
};
