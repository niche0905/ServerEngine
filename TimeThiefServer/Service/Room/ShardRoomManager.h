#pragma once

class Room;

/*--------------------
   ShardRoomManager
--------------------*/
//
// ShardRoomManager는 Shard 내에서 Room을 관리하는 클래스입니다.
// Shard 기반이므로 MultiThread 환경에서 Data Race가 없을 것 이므로 lock을 사용하지 않습니다.
//

class ShardRoomManager
{
public:
   using RoomRef = std::shared_ptr<Room>;
   
public:
   ShardRoomManager() = default;
   ~ShardRoomManager() = default;
   
   ShardRoomManager(ShardRoomManager const&) = delete;
   ShardRoomManager& operator=(ShardRoomManager const&) = delete;
   
public:
   bool AddRoom(RoomId roomId, RoomRef room);
   bool RemoveRoom(RoomId roomId);
   RoomRef FindRoom(RoomId roomId) const;
   
   size_t GetRoomCount() const;
   
   std::vector<RoomRef> GetRoomSnapshot() const;
   
   void Clear();
   
private:
   std::unordered_map<RoomId, RoomRef> rooms_;
    
};
