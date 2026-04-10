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
   auto poppedIds = queue_.TryPopMatch(matchSize_);
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
   
   std::vector<PlayerId> requeueIds;
   requeueIds.reserve(poppedIds.size());

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
   
   // Match Fail (Valid Player Under Size)
   if (candidates.size() < matchSize_) {
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
   params.playerIds = poppedIds;
   
   bool ok = shardManager_->RequestCreateRoom(shardId, std::move(params));
   if (not ok) {
      // Room Create Request Fail
      
      for (const auto& candidate : candidates)
         requeueIds.push_back(candidate.player->id_);
      
      queue_.RequeueFrontBatch(requeueIds);
   }
}
