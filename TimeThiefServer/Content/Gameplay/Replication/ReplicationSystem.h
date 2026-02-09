#pragma once
#include "ReplicationTypes.h"
#include "ReplicatedState.h"
#include "ReplicationEvent.h"
#include "InterestManager.h"
#include <unordered_map>
#include <unordered_set>

// 실제 네트워크 송신은 추후에 구현
class IRepSink
{
public:
   virtual ~IRepSink() = default;
   
   virtual void SendSpawn(ClientId to, RepObjectId obj) = 0;
   virtual void SendDespawn(ClientId to, RepObjectId obj) = 0;
   
   virtual void SendState(ClientId to, RepObjectId obj, RepMask dirtyMask) = 0;
   virtual void SendEvent(ClientId to, const RepEvent& event) = 0;
};

/*---------------------
   ReplicationSystem
---------------------*/
//
// ReplicationSystem는 게임 오브젝트의 상태를 클라이언트에 동기화하는 역할을 합니다.
//

class ReplicationSystem
{
public:
   explicit ReplicationSystem(IRepSink& sink)
      : sink_(sink)
   {
   }
   
   void RegisterObject(RepObjectId obj);
   void UnregisterObject(RepObjectId obj);
   
   void MarkDirty(RepObjectId obj, RepField f);
   
   void PushEvent(const RepEvent& event);
   
   template<typename TRoom>
   void Flush(const TRoom& room, uint64 nowMs)
   {
      frame_.seq++;
      frame_.nowMs = nowMs;
      
      InterestResult interest = interestManager_.BuildInterest(room);      // 매번 계산해야 할지는 고민...
      
      // spawn/despawn 처리
      FlushSpawnDespawn(interest);
      // 상태 동기화 처리
      FlushState(interest);
      // 이벤트 처리
      FlushEvents(interest);
      
      // 이벤트 정리
      events_.Clear();
   }
   
private:
   void FlushSpawnDespawn(const InterestResult& interest);
   void FlushState(const InterestResult& interest);
   void FlushEvents(const InterestResult& interest);
   
private:
   IRepSink& sink_;
   InterestManager interestManager_;
   RepFrame frame_{};
   
   // 복제 관리 오브젝트 집합
   std::unordered_map<RepObjectId, ReplicatedState> objects_;
   
   // 각 클라이언트가 마지막으로 본 오브젝트 집합
   std::unordered_map<ClientId, InterestSet> lastVisible_;
   
   ReplicationEventQueue events_;
    
};
