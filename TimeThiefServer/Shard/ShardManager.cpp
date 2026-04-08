#include "pch.h"
#include "ShardManager.h"
#include "GameShard.h"
#include "RoomDirectory.h"
#include "Core/Thread/ThreadManager.h"
#include "Service/Room/CreateRoomParams.h"

/*----------------
   ShardManager
----------------*/

bool ShardManager::Init(int32 shardCount, SessionManager* sessionManager, RoomDirectory* roomDirectory)
{
   if (shardCount <= 0 or roomDirectory == nullptr)
      return false;
   
   roomDirectory_ = roomDirectory;
   sessionManager_ = sessionManager;
   shards_.clear();
   shards_.reserve(shardCount);
   
   for (int32 i = 0; i < shardCount; ++i) {
      ShardId shardId = static_cast<ShardId>(i + 1);
      shards_.push_back(std::make_unique<GameShard>(shardId, *sessionManager_, *roomDirectory_));
   }
   
   return true;
}

bool ShardManager::Start(ThreadManager& threadManager)
{
   for (auto& shard : shards_) {
      if (!shard or !shard->Start(threadManager)) {
         return false;
      }
   }
   
   return true;
}

void ShardManager::Stop()
{
   for (auto& shard : shards_) {
      if (shard) {
         shard->Stop();
      }
   }
}

GameShard* ShardManager::GetShard(ShardId shardId)
{
   if (shardId == 0)
      return nullptr;
   
   const size_t index = static_cast<size_t>(shardId - 1);
   if (index >= shards_.size())
      return nullptr;
   
   return shards_[index].get();
}

const GameShard* ShardManager::GetShard(ShardId shardId) const
{
   if (shardId == 0)
      return nullptr;
   
   const size_t index = static_cast<size_t>(shardId - 1);
   if (index >= shards_.size())
      return nullptr;
   
   return shards_[index].get();
}

bool ShardManager::Enqueue(ShardId shardId, Job job)
{
   GameShard* shard = GetShard(shardId);
   if (!shard)
      return false;
   
   return shard->Enqueue(std::move(job));
}

bool ShardManager::RequestCreateRoom(ShardId shardId, CreateRoomParams params)
{
   GameShard* shard = GetShard(shardId);
   if (!shard)
      return false;
   
   return Enqueue(shardId, [this, shardId, params = std::move(params)]()
   {
      GameShard* shard = GetShard(shardId);
      if (!shard)
         return;
      
      shard->CreateRoom(std::move(params));
   });
}

ShardId ShardManager::SelectShardForNewRoom()
{
   if (shards_.empty())
      return 0;   // Shard가 없는 경우, 기본적으로 Shard ID 0을 반환 (이 경우는 발생하지 않아야 함)
   
   uint32 index = nextShard_.fetch_add(1, std::memory_order_relaxed) % shards_.size();
   return static_cast<ShardId>(index + 1);
}

int32 ShardManager::GetShardCount() const
{
   return static_cast<int32>(shards_.size());
}
