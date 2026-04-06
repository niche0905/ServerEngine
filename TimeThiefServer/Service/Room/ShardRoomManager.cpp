#include "pch.h"
#include "ShardRoomManager.h"
#include "Room.h"

/*--------------------
   ShardRoomManager
--------------------*/

bool ShardRoomManager::AddRoom(RoomId roomId, RoomRef room)
{
   if (roomId == 0 or room == nullptr)
      return false;
   
   auto [it, inserted] = rooms_.emplace(roomId, std::move(room));
   return inserted;
}

bool ShardRoomManager::RemoveRoom(RoomId roomId)
{
   if (roomId == 0)
      return false;
   
   return rooms_.erase(roomId) > 0;
}

ShardRoomManager::RoomRef ShardRoomManager::FindRoom(RoomId roomId) const
{
   if (roomId == 0)
      return nullptr;
   
   auto it = rooms_.find(roomId);
   if (it == rooms_.end()) {
      return nullptr;
   }
   
   return it->second;
}

size_t ShardRoomManager::GetRoomCount() const
{
   return rooms_.size();
}

std::vector<ShardRoomManager::RoomRef> ShardRoomManager::GetRoomSnapshot() const
{
   std::vector<RoomRef> snapshot;
   snapshot.reserve(rooms_.size());
   
   for (const auto& [roomId, room] : rooms_) {
      snapshot.push_back(room);
   }
   
   return snapshot;
}

void ShardRoomManager::Clear()
{
   rooms_.clear();
}
