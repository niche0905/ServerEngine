#pragma once
#include <shared_mutex>
#include "Service/Player/Player.h"

/*-----------------
   PlayerManager
-----------------*/
//
// PlayerManager는 모든 플레이어를 관리합니다.
//

class PlayerManager
{
public:
   using PlayerRef = std::shared_ptr<Player>;
   
public:
   PlayerRef Create(PlayerId playerId);      // 플레이어 없다면 생성/반환, 이미 존재하면 기존 플레이어 반환
   void Remove(PlayerId playerId);
   
   void Clear();
   
   PlayerRef Find(PlayerId playerId) const;
   size_t GetPlayerCount() const;
   
   void UpdateRoute(PlayerId playerId, ShardId shardId, RoomId roomId);
   
   std::vector<PlayerRef> SnapshotPlayers() const;
   
private:
   mutable std::shared_mutex mutex_;                           // 플레이어 리스트 보호용 뮤텍스 (읽기가 많을 것으로 예상되어 shared_mutex 사용)
   
   std::unordered_map<PlayerId, PlayerRef> playersById_;       // 플레이어 ID -> 플레이어 참조
    
};
