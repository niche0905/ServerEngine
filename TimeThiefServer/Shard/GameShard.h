#pragma once
#include <atomic>
#include <memory>
#include <functional>
#include "Service/Timer/TimerTask.h"
#include "Service/Timer/TimerQueue.h"
#include "Service/Job/JobQueue.h"
#include "Service/Room/CreateRoomParams.h"
#include "Service/Room/ShardRoomManager.h"

class RoomDirectory;
class Room;
class ThreadManager;
class SessionManager;

/*--------------
   GameShard
--------------*/
//
// GameShard는 한 Shard에 필요한 정보드를 담고 관리하는 클래스입니다.
// Job Queue 처리와 Room Tick을 수행합니다
//

// TODO: Shard는 Thread 관련한 내용이라 Cache Thrashing을 발생하게 하지 않기 위해 64바이트 정렬을 하는 것이 좋아 보임
//       혹은 Shard Manager에서 vector를 사용할 때 패딩을 주는 방법도 찾아 볼 것
class GameShard
{
public:
   using RoomRef = std::shared_ptr<Room>;
   
public:
   explicit GameShard(ShardId shardId, SessionManager& sessionManager, RoomDirectory& roomDirectory);
   
   bool Start(ThreadManager& threadManager);
   void Stop();
   
   void Run();
   
   bool Enqueue(Job job);
   
   bool CreateRoom(CreateRoomParams params);
   bool AddRoom(RoomId roomId, RoomRef room);
   bool RemoveRoom(RoomId roomId);
   RoomRef FindRoom(RoomId roomId) const;
   
   size_t GetRoomCount() const;
   
   ShardId GetShardId() const { return shardId_; }
   
public:
   TimerId ScheduleAt(TimePoint executeAt, Job job);
   TimerId ScheduleAfter(Duration delay, Job job);
   bool CancelTimer(TimerId timerId);
   
private:
   void ProcessJobs();
   void ProcessTimers();
   void TickRooms();
   
private:
   SessionManager&         sessionManager_;
   RoomDirectory&          roomDirectory_;
   
private:
   ShardId                 shardId_ = 0;
   std::atomic<bool>       running_ = false;
   
   ShardRoomManager        shardRoomManager_{};
   JobQueue                jobQueue_{};
   TimerQueue              timerQueue_{};
    
};
