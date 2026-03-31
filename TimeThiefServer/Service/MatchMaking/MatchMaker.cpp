#include "pch.h"
#include "MatchMaker.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Service/Player/PlayerManager/PlayerManager.h"
#include "Service/Room/Room.h"
#include "Service/Room/RoomManager.h"

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

MatchMaker g_MatchMaker;

bool MatchMaker::Enqueue(PlayerId playerId)
{
   bool success = queue_.Enqueue(playerId);
   
   if (success) {
      auto session = g_SessionManager.FindByPlayerId(playerId);
      
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
      auto session = g_SessionManager.FindByPlayerId(playerId);
      
      if (session) {
         session->SetState(PlayerSessionState::InLobby);
      }
   }
   
   return success;
}

void MatchMaker::TryMatch()
{
   auto poppedIds = queue_.TryPopMatch(kMatchSize);
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
      auto player = g_PlayerManager.Find(playerId);
      if (!IsValidPlayer(player))
         continue;

      auto session = g_SessionManager.FindByPlayerId(playerId);
      if (!IsValidSession(session))
         continue;

      candidates.push_back({ player, session });
   }
   
   // Match Fail (Valid Player Under Size)
   if (candidates.size() < kMatchSize) {
      for (const auto& candidate : candidates)
         requeueIds.push_back(candidate.player->id_);
      
      queue_.RequeueFrontBatch(requeueIds);
      return;
   }
   
   // Room Create
   auto room = g_RoomManager->CreateRoom();
   if (!room) {   // Room Create Fail
      for (const auto& candidate : candidates)
         requeueIds.push_back(candidate.player->id_);
      
      queue_.RequeueFrontBatch(requeueIds);
      return;
   }
   
   // Matching Success
   se::lobby::N_MatchFound matchFoundPkt;
   matchFoundPkt.set_room_id(room->GetRoomId());
   auto sendBuffer = ServerPacketHandler::MakeSendBuffer(matchFoundPkt);
   if (!sendBuffer) {   // Packet Create Fail
      for (const auto& candidate : candidates)
         requeueIds.push_back(candidate.player->id_);
      
      queue_.RequeueFrontBatch(requeueIds);
      return;
   }
   
   for (auto& c : candidates) {
      
      c.session->SetState(PlayerSessionState::MatchingSucc);
      c.session->Send(sendBuffer);
   }
}
