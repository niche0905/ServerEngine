#include "pch.h"
#include "ShardManager.h"
#include <random>
#include "GameShard.h"
#include "RoomDirectory.h"
#include "Core/Thread/ThreadManager.h"
#include "Service/Room/CreateRoomParams.h"

/*----------------
   ShardManager
----------------*/

bool ShardManager::Init(int32 shardCount, SessionManager* sessionManager, RoomDirectory* roomDirectory, PlayerManager* playerManager, GameDataManager* gameDataManager, const GameConfig& gameConfig)
{
   if (shardCount <= 0 or sessionManager == nullptr or roomDirectory == nullptr or playerManager == nullptr or gameDataManager == nullptr)
      return false;
   
   sessionManager_ = sessionManager;
   playerManager_ = playerManager;
   roomDirectory_ = roomDirectory;
   gameDataManager_ = gameDataManager;
   shards_.clear();
   shards_.reserve(shardCount);
   
   for (int32 i = 0; i < shardCount; ++i) {
      ShardId shardId = static_cast<ShardId>(i + 1);
      shards_.push_back(std::make_unique<GameShard>(shardId, *sessionManager_, *roomDirectory_, *playerManager_, *gameDataManager_, gameConfig));
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
   
   // P2C (Power of Two Choices)
   {
      const uint32 shardCount = static_cast<uint32>(shards_.size());
      
      static thread_local std::mt19937 rng(std::random_device{}());
      std::uniform_int_distribution<uint32> dist(0, shardCount - 1);
      
      uint32 a = dist(rng);
      uint32 b = dist(rng);
      
      if (a == b) {
         b = (b + 1) % shardCount;  // 같은 Shard가 선택된 경우, 다음 Shard로 선택
      }
      
      // 아래 코드는 당장 사용할 수 없음 (GetRoomCount는 Multi Thread를 고려하지 않았기에 정상 작동 불가함)
      // 사용하고자 하면 atomic 한 counter를 두고 그 값을 사용하여야 함 (Thread Safe하게)
      size_t sizeA = shards_[a]->GetRoomCount();
      size_t sizeB = shards_[b]->GetRoomCount();
      
      uint32 selected = (sizeA <= sizeB) ? a : b;
      return static_cast<ShardId>(selected + 1);
   }
}

int32 ShardManager::GetShardCount() const
{
   return static_cast<int32>(shards_.size());
}
