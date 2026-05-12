#include "pch.h"
#include "MatchMaker.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Service/Player/PlayerManager/PlayerManager.h"
#include "Service/Room/CreateRoomParams.h"
#include "Service/Room/Room.h"
#include "Service/Room/RoomIdGenerator.h"
#include "Service/Room/RoomManager.h"
#include "Shard/RoomDirectory.h"
#include "Shard/ShardManager.h"

namespace 
{
   static bool IsValidPlayer(std::shared_ptr<Player> player)
   {
      if (player == nullptr)
         return false;
      
      if (player->sessionId_ == 0)
         return false;
      
      if (player->roomId_ != 0)
         return false;
      
      return true;
   }
   
   static bool IsValidSession(std::shared_ptr<PlayerSession> session)
   {
      if (session == nullptr)
         return false;
      
      if (!session->IsConnected())
         return false;
      
      // TODO: PlayerSession State 확장
      if (session->GetState() != PlayerSessionState::MatchMaking)
         return false;
      
      return true;
   }
   
}

/*--------------
   MatchMaker
--------------*/

bool MatchMaker::Init(SessionManager& sessionManager, PlayerManager& playerManager, ShardManager& shardManager, RoomDirectory& roomDirectory, RoomIdFactory roomIdFactory, size_t matchSize)
{
   sessionManager_ = &sessionManager;
   playerManager_ = &playerManager;
   shardManager_ = &shardManager;
   roomDirectory_ = &roomDirectory;
   roomIdFactory_ = std::move(roomIdFactory);
   
   matchSize_ = matchSize;
   
   return sessionManager_ && playerManager_ && shardManager_ && roomDirectory_ && roomIdFactory_;
}

void MatchMaker::SetPartialMatch(bool enable, Duration waitTime, size_t partialMatchSize)
{
   enablePartialMatch_ = enable;
   partialMatchWaitTime_ = waitTime;
   minPartialMatchSize_ = partialMatchSize;
   
   if (minPartialMatchSize_ > matchSize_) {
      consoleLogger->Log(Color::Yellow, L"[MatchMaker] Partial match size cannot be greater than full match size. Adjusting partial match size to match size.\n");
      minPartialMatchSize_ = matchSize_;
   }
}

bool MatchMaker::Enqueue(PlayerId playerId)
{
   bool success = queue_.Enqueue(playerId);
   
   if (success) {
      auto session = sessionManager_->FindByPlayerId(playerId);
      
      if (session) {
         session->SetState(PlayerSessionState::MatchMaking);
      }
   }
   
   return success;
}

bool MatchMaker::Cancel(PlayerId playerId)
{
   bool success = queue_.Cancel(playerId);
   
   if (success) {
      auto session = sessionManager_->FindByPlayerId(playerId);
      
      if (session) {
         session->SetState(PlayerSessionState::InLobby);
      }
   }
   
   return success;
}

void MatchMaker::TryMatch()
{
   const size_t queueSize = queue_.Size();
   
   if (queueSize < minPartialMatchSize_) 
      return;  // 매칭 시도 조건 미달 (매칭 시도 자체를 하지 않음)
   
   size_t targetMatchSize = matchSize_;
   
   if (queueSize < matchSize_) {
      if (!enablePartialMatch_)  // 불완전 매칭 기능이 비활성화된 경우, match size에 미달하면 매칭 시도 자체를 하지 않음
         return;
      
      const auto lastEnterTime = queue_.GetLastEnterTime();
      if (!lastEnterTime.has_value())
         return;  // 큐에 플레이어가 존재하지만 마지막 입장 시간이 없는 경우 (정상적이지 않은 상황), 매칭 시도 자체를 하지 않음
      
      auto elapsed = Clock::now() - lastEnterTime.value();
      if (elapsed < partialMatchWaitTime_)  // 불완전 매칭 대기 시간 미경과, 매칭 시도 자체를 하지 않음
         return;
      
      targetMatchSize = queueSize;
   }
   
   auto poppedIds = queue_.TryPopMatch(targetMatchSize);
   if (poppedIds.empty())
      return;
   
   struct MatchCandidate
   {
      std::shared_ptr<Player> player;
      std::shared_ptr<PlayerSession> session;
   };
   
   // Player Validate
   std::vector<MatchCandidate> candidates;
   candidates.reserve(poppedIds.size());

   for (PlayerId playerId : poppedIds)
   {
      auto player = playerManager_->Find(playerId);
      if (!IsValidPlayer(player))
         continue;

      auto session = sessionManager_->FindByPlayerId(playerId);
      if (!IsValidSession(session))
         continue;

      candidates.push_back({ player, session });
   }
   
   // Match Fail (Valid Player Under Partial Size)
   if (candidates.size() < minPartialMatchSize_) {
      std::vector<PlayerId> requeueIds;
      for (const auto& candidate : candidates)
         requeueIds.push_back(candidate.player->id_);
      
      queue_.RequeueFrontBatch(requeueIds);
      return;
   }
   
   // Room Create
   RoomId roomId = roomIdFactory_();
   ShardId shardId = shardManager_->SelectShardForNewRoom();
   
   CreateRoomParams params;
   params.roomId = roomId;
   
   params.playerIds.reserve(candidates.size());
   for (const auto& candidate : candidates)
      params.playerIds.push_back(candidate.player->id_);
   
   bool ok = shardManager_->RequestCreateRoom(shardId, std::move(params));
   if (not ok) {
      // Room Create Request Fail
      
      std::vector<PlayerId> requeueIds;
      for (const auto& candidate : candidates)
         requeueIds.push_back(candidate.player->id_);
      
      queue_.RequeueFrontBatch(requeueIds);
   }
}
