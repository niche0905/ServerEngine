#pragma once
#include <memory>
#include <vector>
#include <atomic>
#include "GameShard.h"

class GameDataManager;
struct CreateRoomParams;
class GameShard;
class RoomDirectory;
class ThreadManager;

/*----------------
   ShardManager
----------------*/
//
// ShardManager는 여러 GameShard 인스턴스를 관리하는 클래스입니다.
// Shard ID를 가지고 Shard에 접근할 수 있고, 접근 뿐만 아니라 Job을 Enqueue하는 기능도 제공합니다.
//

class ShardManager
{
public:
   ShardManager() = default;
   ~ShardManager() = default;
   
   ShardManager(ShardManager const&) = delete;
   ShardManager& operator=(ShardManager const&) = delete;
   
public:
   bool Init(int32 shardCount, SessionManager* sessionManager, RoomDirectory* roomDirectory, GameDataManager* gameDataManager, const GameConfig& gameConfig);
   bool Start(ThreadManager& threadManager);
   void Stop();
   
   GameShard* GetShard(ShardId shardId);
   const GameShard* GetShard(ShardId shardId) const;
   
   bool Enqueue(ShardId shardId, Job job);
   bool RequestCreateRoom(ShardId shardId, CreateRoomParams params);
   
   ShardId SelectShardForNewRoom();
   
   int32 GetShardCount() const;
   
private:
   SessionManager* sessionManager_ = nullptr;    // non-owning
   RoomDirectory* roomDirectory_ = nullptr;      // non-owning
   GameDataManager* gameDataManager_ = nullptr;  // non-owning
   
private:
   std::vector<std::unique_ptr<GameShard>> shards_;
   std::atomic<uint32> nextShard_ = 0;   // 라운드로빈 방식으로 Shard를 선택하기 위한 카운터
    
};
