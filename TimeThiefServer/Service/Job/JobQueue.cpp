#include "pch.h"
#include "JobQueue.h"

/*------------
   JobQueue
------------*/

bool JobQueue::Push(Job job)
{
   if (!job) return false;
   
   {
      std::lock_guard<std::mutex> lock(mutex_);
      queue_.push(std::move(job));
   }
   
   return true;
}

bool JobQueue::TryPop(Job& outJob)
{
   std::lock_guard<std::mutex> lock(mutex_);
   if (queue_.empty()) return false;
   
   outJob = std::move(queue_.front());
   queue_.pop();
   return true;
}

size_t JobQueue::DrainTo(std::vector<Job>& outJobs)
{
   std::lock_guard<std::mutex> lock(mutex_);
   size_t count = 0;
   while (!queue_.empty()) {
      outJobs.push_back(std::move(queue_.front()));
      queue_.pop();
      ++count;
   }
   return count;
}
