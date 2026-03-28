#pragma once
#include <atomic>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include "Content/Player/Player.h"

class Room;

/*----------------
   RoomManager
----------------*/
//
// RoomManager는 게임의 방(Room)을 관리하는 클래스입니다. 각 방은 플레이어들이 모여서 게임을 진행하는 공간입니다.
//

class RoomManager
{
public:
   using RoomRef = std::shared_ptr<Room>;
   
public:
   RoomManager() = default;
   ~RoomManager() = default;
   
public:
   RoomRef CreateRoom(RoomId roomId);
   
   bool RemoveRoom(RoomId roomId);
   RoomRef FindRoom(RoomId roomId) const;
   
   size_t GetRoomCount() const;
   
   std::vector<RoomRef> GetRoomSnapshot() const;
   
   void Clear();
   
private:
   RoomId GenerateUniqueRoomId();
   
private:
   mutable std::mutex mutex_;
   std::unordered_map<RoomId, RoomRef> rooms_;
   std::atomic<RoomId> nextRoomId_;
    
};
