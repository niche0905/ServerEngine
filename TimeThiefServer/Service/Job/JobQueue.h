#pragma once

/*------------
   JobQueue
------------*/
//
// JobQueue는 서버에서 실행되어야 하는 작업들을 관리하는 큐입니다.
// MPSC (Multi-Producer, Single-Consumer) 방식으로 구현을 고려하며
// 현재는 Lock을 통해서 구현할 예정이지만, 추후에 Lock-Free 방식으로 개선할 수 있습니다.
//

class JobQueue
{
public:
   bool Push(Job job);
   bool TryPop(Job& outJob);
   
   size_t DrainTo(std::vector<Job>& outJobs);
   
private:
   std::mutex mutex_;
   std::queue<Job> queue_;
    
};
