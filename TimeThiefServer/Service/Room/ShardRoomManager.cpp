#include "pch.h"
#include "ShardRoomManager.h"
#include "Room.h"

/*--------------------
   ShardRoomManager
--------------------*/

ShardRoomManager::RoomRef ShardRoomManager::CreateRoom(RoomId roomId)
{
   auto room = Room::Create(roomId);
   
   auto [it, inserted] = rooms_.emplace(roomId, room);
   if (!inserted) {
      return nullptr;
   }
   
   return room;
}

bool ShardRoomManager::RemoveRoom(RoomId roomId)
{
   return rooms_.erase(roomId) > 0;
}

ShardRoomManager::RoomRef ShardRoomManager::FindRoom(RoomId roomId) const
{
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
