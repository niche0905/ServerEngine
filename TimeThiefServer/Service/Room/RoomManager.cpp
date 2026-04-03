#include "pch.h"
#include "RoomManager.h"
#include "Room.h"

/*----------------
   RoomManager
----------------*/

RoomManager* g_RoomManager = new RoomManager();

RoomManager::RoomRef RoomManager::CreateRoom()
{
   RoomId roomId = GenerateUniqueRoomId();
   auto room = Room::Create(roomId);
   
   {
      std::lock_guard lock(mutex_);
      auto [it, inserted] = rooms_.emplace(roomId, room);
      if (!inserted) {
         return nullptr;
      }
   }
   
   return room;
}

RoomManager::RoomRef RoomManager::CreateRoom(RoomId roomId)
{
   auto room = Room::Create(roomId);
   
   {
      std::lock_guard lock(mutex_);
      auto [it, inserted] = rooms_.emplace(roomId, room);
      if (!inserted) {
         return nullptr;
      }
   }
   
   return room;
}

bool RoomManager::RemoveRoom(RoomId roomId)
{
   std::lock_guard lock(mutex_);
   return rooms_.erase(roomId) > 0;
}

RoomManager::RoomRef RoomManager::FindRoom(RoomId roomId) const
{
   std::lock_guard lock(mutex_);
   
   auto it = rooms_.find(roomId);
   if (it == rooms_.end()) {
      return nullptr;
   }
   
   return it->second;
}

size_t RoomManager::GetRoomCount() const
{
   std::lock_guard lock(mutex_);
   return rooms_.size();
}

std::vector<RoomManager::RoomRef> RoomManager::GetRoomSnapshot() const
{
   std::vector<RoomRef> snapshot; 
   
   std::lock_guard lock(mutex_);
   snapshot.reserve(rooms_.size());
   
   for (const auto& [roomId, room] : rooms_) {
      snapshot.push_back(room);
   }
   
   return snapshot;
}

void RoomManager::Clear()
{
   std::lock_guard lock(mutex_);
   rooms_.clear();
}

RoomId RoomManager::GenerateUniqueRoomId()
{
   return nextRoomId_.fetch_add(1, std::memory_order_relaxed);
}
