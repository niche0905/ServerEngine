#include "pch.h"
#include "MatchMakingQueue.h"

/*--------------------
   MatchMakingQueue
--------------------*/

bool MatchMakingQueue::Enqueue(PlayerId playerId)
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   if (waitingMap_.contains(playerId))
      return false;
   
   waitingList_.push_back(playerId);
   auto it = std::prev(waitingList_.end());
   waitingMap_.emplace(playerId, it);
   
   return true;
}

bool MatchMakingQueue::Cancel(PlayerId playerId)
{
   std::lock_guard<std::mutex> lock(mutex_);
   
   auto it = waitingMap_.find(playerId);
   if (it == waitingMap_.end())
      return false;
   
   waitingList_.erase(it->second);
   waitingMap_.erase(it);
   
   return true;
}

bool MatchMakingQueue::RequeueFront(PlayerId playerId)
{
   std::lock_guard<std::mutex> lock(mutex_);

   if (waitingMap_.contains(playerId))
      return false;

   waitingList_.push_front(playerId);
   waitingMap_[playerId] = waitingList_.begin();
   return true;
}

void MatchMakingQueue::RequeueFrontBatch(const std::vector<PlayerId>& playerIds)
{
   std::lock_guard<std::mutex> lock(mutex_);

   for (auto it = playerIds.rbegin(); it != playerIds.rend(); ++it)
   {
      PlayerId playerId = *it;

      if (waitingMap_.contains(playerId))
         continue;

      waitingList_.push_front(playerId);
      waitingMap_[playerId] = waitingList_.begin();
   }
}

std::vector<PlayerId> MatchMakingQueue::TryPopMatch(size_t matchSize)
{
   std::vector<PlayerId> result;
   if (matchSize == 0)
      return result;
   
   std::lock_guard<std::mutex> lock(mutex_);
   
   if (waitingList_.size() < matchSize)
      return result;
   
   result.reserve(matchSize);
   
   for (size_t i = 0; i < matchSize; ++i) {
      PlayerId playerId = waitingList_.front();
      waitingList_.pop_front();
      waitingMap_.erase(playerId);
      result.push_back(playerId);
   }
   return result;
}

bool MatchMakingQueue::IsWaiting(PlayerId playerId) const
{
   std::lock_guard<std::mutex> lock(mutex_);
   return waitingMap_.contains(playerId);
}

size_t MatchMakingQueue::Size() const
{
   std::lock_guard<std::mutex> lock(mutex_);
   return waitingList_.size();
}
