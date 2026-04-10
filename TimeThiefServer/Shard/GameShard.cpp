#include "pch.h"
#include "GameShard.h"
#include "RoomDirectory.h"
#include "Core/Thread/ThreadManager.h"
#include "Network/ServerConfig.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Service/Room/Room.h"

/*--------------
   GameShard
--------------*/

GameShard::GameShard(ShardId shardId, SessionManager& sessionManager, RoomDirectory& roomDirectory, const GameDataManager& gameDataManager, const GameConfig& config)
   : shardId_{ shardId }
   , sessionManager_{ sessionManager }
   , roomDirectory_{ roomDirectory }
   , gameDataManager_{ gameDataManager }
   , gameConfig_{ config }
   , roomTickIntervalMs_{ config.roomTickIntervalMs }
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
      ProcessTimers();
      ProcessRoomTicks();
      
      std::this_thread::sleep_for(std::chrono::milliseconds(1));
   }
}

bool GameShard::Enqueue(Job job)
{
   if (!job)
      return false;
   
   jobQueue_.Push(std::move(job));
   return true;
}

bool GameShard::CreateRoom(CreateRoomParams params)
{
   if (params.roomId == 0)
      return false;
   
   auto room = Room::Create(params.roomId, sessionManager_);
   if (!room) {
      // TODO: MatchMaking Fail 처리 해야 할 듯 싶다 (다시 등록 or 매칭 실패 패킷 보내기)
      return false;
   }
   
   room->Init(this, gameDataManager_, gameConfig_);
   room->SetPlayer(params.playerIds);
   
   // TODO: Room 초기화 (스크립트 읽어와서 Spawn 및 세팅 하는게 좋을듯)
   if (!AddRoom(params.roomId, room)) {
      // TODO: MatchMaking Fail 처리 해야 할 듯 싶다 (다시 등록 or 매칭 실패 패킷 보내기)
      return false;
   }
   
   
   roomDirectory_.RegisterRoom(params.roomId, shardId_);
   
   se::lobby::N_MatchFound matchFoundPkt;
   matchFoundPkt.set_room_id(params.roomId);
   auto sendBuffer = ServerPacketHandler::MakeSendBuffer(matchFoundPkt);
   if (sendBuffer) {
      for (const auto& playerId : params.playerIds) {
         auto session = sessionManager_.FindByPlayerId(playerId);
         if (session) {
            session->SetState(PlayerSessionState::MatchingSucc);
            session->Send(sendBuffer);
         }
      }
   }
   
   return true;
}

bool GameShard::AddRoom(RoomId roomId, RoomRef room)
{
   if (!shardRoomManager_.AddRoom(roomId, std::move(room)))
      return false;
   
   return true;
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

TimerId GameShard::ScheduleAt(TimePoint executeAt, Job job)
{
   return timerQueue_.ScheduleAt(executeAt, job);
}

TimerId GameShard::ScheduleAfter(Duration delay, Job job)
{
   return timerQueue_.ScheduleAfter(delay, job);
}

bool GameShard::CancelTimer(TimerId timerId)
{
   return timerQueue_.Cancel(timerId);
}

void GameShard::ProcessJobs()
{
   std::vector<Job> jobs;
   jobs.reserve(64);
   size_t jobCount = jobQueue_.DrainTo(jobs);

   for (auto& job : jobs) {
      if (job) {
         job();
      }
   }
}

void GameShard::ProcessTimers()
{
   std::vector<Job> jobs;    // TODO: vector 생성을 매번 하지 않고 scratch buffer 같은 걸로 재활용하는 방법도 좋을 듯 하다
   jobs.reserve(64);
   timerQueue_.PopExpired(Clock::now(), jobs);
   
   // 실행 전에 Timer 관련 metadata를 보고 싶은 경우는 TimerTask를 받아오도록 수정
   for (auto& job : jobs) {
      if (job) {
         job();
      }
   }
}

void GameShard::ProcessRoomTicks()
{
   const TimePoint now = Clock::now();
   
   ScheduledRoomTick roomTick;
   while (roomScheduler_.PopDue(now, roomTick)){
      auto room = shardRoomManager_.FindRoom(roomTick.roomId);
      if (!room)
         continue;
      
      room->UpdateTick(roomTickIntervalMs_);
      
      roomScheduler_.Schedule(roomTick.roomId, roomTick.executeAt + roomTickIntervalMs_);
   }
}

void GameShard::ScheduleRoomFirstTick(RoomId roomId)
{
   roomScheduler_.Schedule(roomId, Clock::now() + roomTickIntervalMs_);
}
