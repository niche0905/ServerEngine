#include "pch.h"
#include "GameShard.h"

#include "Core/Thread/ThreadManager.h"

/*--------------
   GameShard
--------------*/

GameShard::GameShard(ShardId shardId)
   : shardId_{ shardId }
{
   
}

bool GameShard::Start(ThreadManager& threadManager)
{
   bool expected = false;
   if (!running_.compare_exchange_strong(expected, true))
      return false;
   
   threadManager.Launch([this]()
   {
      Run();
   });
   
   return true;
}

void GameShard::Stop()
{
   running_.store(false);
}

void GameShard::Run()
{
   while (running_.load()) {
      ProcessJobs();
      TickRooms();
      
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   }
}

bool GameShard::Enqueue(Job job)
{
   if (!job)
      return false;
   
   // TODO: Job queue push
   return true;
}

bool GameShard::AddRoom(RoomId roomId, RoomRef room)
{
   return shardRoomManager_.AddRoom(roomId, std::move(room));
}

bool GameShard::RemoveRoom(RoomId roomId)
{
   return shardRoomManager_.RemoveRoom(roomId);
}

GameShard::RoomRef GameShard::FindRoom(RoomId roomId) const
// 이거 사용할 때 같은 샤드가 아니라면 주의 해야 함...
{
   return shardRoomManager_.FindRoom(roomId);
}

size_t GameShard::GetRoomCount() const
{
   return shardRoomManager_.GetRoomCount();
}

void GameShard::ProcessJobs()
{
   // TODO: Job Queue Logic
}

void GameShard::TickRooms()
{
   // TEMP
   auto rooms = shardRoomManager_.GetRoomSnapshot();
   
   for (const auto& room : rooms) {
      if (!room)
         continue;
      
      // room->Tick();
   }
   // TODO: 이 방식이 아니라 Timer Queue에 따라 Tick이 필요한 Room들만 Tick 하는 방식으로 변경하고 싶다
}
