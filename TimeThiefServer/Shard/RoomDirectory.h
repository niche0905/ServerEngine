#pragma once
#include <optional>
#include <shared_mutex>
#include <unordered_map>

/*-----------------
   RoomDirectory
-----------------*/
//
// RoomDirectory는 Room에 맞는 Shard를 찾아주는 역할을 합니다.
//

class RoomDirectory
{
public:
   bool RegisterRoom(RoomId roomId, ShardId shardId);
   bool UnregisterRoom(RoomId roomId);
   std::optional<ShardId> FindShardId(RoomId roomId) const;
   
private:
   mutable std::shared_mutex mutex_;
   std::unordered_map<RoomId, ShardId> roomToShard_;
    
};
