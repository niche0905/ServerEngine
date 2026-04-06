#pragma once
#include <memory>
#include <vector>
#include <atomic>

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
   bool Init(int32 shardCount, RoomDirectory* roomDirectory);
   bool Start(ThreadManager& threadManager);
   void Stop();
   
   GameShard* GetShard(ShardId shardId);
   const GameShard* GetShard(ShardId shardId) const;
   
   bool Enqueue(ShardId shardId, Job job);
   
   ShardId SelectShardForNewRoom();
   
   int32 GetShardCount() const;
   
private:
   std::vector<std::unique_ptr<GameShard>> shards_;
   RoomDirectory* roomDirectory_ = nullptr;
   std::atomic<uint32> nextShard_ = 0;   // 라운드로빈 방식으로 Shard를 선택하기 위한 카운터
    
};
