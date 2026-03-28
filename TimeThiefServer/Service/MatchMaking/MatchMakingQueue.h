#pragma once
#include "Service/Player/Player.h"
#include <mutex>
#include <list>
#include <unordered_map>

/*--------------------
   MatchMakingQueue
--------------------*/
//
// MatchMakingQueue는 매칭에 대한 대기열(자료구조)를 담당하는 클래스이다.
//

class MatchMakingQueue
{
public:
    MatchMakingQueue() = default;
    
    MatchMakingQueue(const MatchMakingQueue&) = delete;
    MatchMakingQueue& operator=(const MatchMakingQueue&) = delete;
    MatchMakingQueue(MatchMakingQueue&&) = delete;
    MatchMakingQueue& operator=(MatchMakingQueue&&) = delete;
    
public:
    bool Enqueue(PlayerId playerId);
    bool Cancel(PlayerId playerId);
    
    bool RequeueFront(PlayerId playerId);
    void RequeueFrontBatch(const std::vector<PlayerId>& playerIds);
    
    std::vector<PlayerId> TryPopMatch(size_t matchSize);
    bool IsWaiting(PlayerId playerId) const;
    
    size_t Size() const;
    
private:
    mutable std::mutex mutex_;
    std::list<PlayerId> waitingList_;
    std::unordered_map<PlayerId, std::list<PlayerId>::iterator> waitingMap_;
    
};
