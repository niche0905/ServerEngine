#include "pch.h"
#include "RoomDirectory.h"

/*-----------------
   RoomDirectory
-----------------*/

bool RoomDirectory::RegisterRoom(RoomId roomId, ShardId shardId)
{
   if (roomId == 0)
      return false;
   
   std::unique_lock lock(mutex_);
   
   auto [it, inserted] = roomToShard_.emplace(roomId, shardId);
   return inserted;
}

bool RoomDirectory::UnregisterRoom(RoomId roomId)
{
   if (roomId == 0)
      return false;
   
   std::unique_lock lock(mutex_);
   
   return roomToShard_.erase(roomId) > 0;
}

std::optional<ShardId> RoomDirectory::FindShardId(RoomId roomId) const
{
   if (roomId == 0)
      return std::nullopt;
   
   std::shared_lock lock(mutex_);
   
   auto it = roomToShard_.find(roomId);
   if (it == roomToShard_.end())
      return std::nullopt;
   
   return it->second;
}
