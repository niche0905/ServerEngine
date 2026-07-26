#pragma once
#include <memory>
#include "Protocol.pb.h"
#include "Routing/PlayerRoute.h"
#include "Shard/ShardManager.h"

struct PlayerRoute;
class Room;
class PacketSession;
class RoomDirectory;
class ShardManager;
class MatchMaker;
class PlayerManager;
class SessionManager;

/*--------------------------
   ServerPacketDispatcher
--------------------------*/
//
// ServerPacketDispatcher는 서버에서 클라이언트로부터 수신된 패킷을 처리하는 역할을 담당하는 클래스입니다.
//

class ServerPacketDispatcher
{
public:
   using PacketSessionRef = std::shared_ptr<PacketSession>;
   
public:
   ServerPacketDispatcher(SessionManager& sessionManager,
      PlayerManager& playerManager,
      MatchMaker& matchMaker,
      RoomDirectory& roomDirectory,
      ShardManager* shardManager = nullptr);
   
public:
   bool Handle_C_HandshakeReq(PacketSessionRef& session, const se::auth::C_HandshakeReq& pkt);
   bool Handle_C_LoginReq(PacketSessionRef& session, const se::auth::C_LoginReq& pkt);
   bool Handle_C_Ping(PacketSessionRef& session, const se::auth::C_Ping& pkt);
   bool Handle_C_SetNicknameReq(PacketSessionRef& session, const se::lobby::C_SetNicknameReq& pkt);
   bool Handle_C_MatchQueueEnterReq(PacketSessionRef& session, const se::lobby::C_MatchQueueEnterReq& pkt);
   bool Handle_C_MatchQueueCancelReq(PacketSessionRef& session, const se::lobby::C_MatchQueueCancelReq& pkt);
   bool Handle_C_RoomEnterReq(PacketSessionRef& session, const se::room::C_RoomEnterReq& pkt);
   bool Handle_C_RoomLeaveReq(PacketSessionRef& session, const se::room::C_RoomLeaveReq& pkt);
   bool Handle_C_LoadingCompleteReq(PacketSessionRef& session, const se::game::C_LoadingCompleteReq& pkt);
   bool Handle_C_MoveReq(PacketSessionRef& session, const se::game::C_MoveReq& pkt);
   bool Handle_C_JumpReq(PacketSessionRef& session, const se::game::C_JumpReq& pkt);
   bool Handle_C_JumpLand(PacketSessionRef& session, const se::game::C_JumpLand& pkt);
   bool Handle_C_DoubleJumpReq(PacketSessionRef& session, const se::game::C_DoubleJumpReq& pkt);
   bool Handle_C_CrouchReq(PacketSessionRef& session, const se::game::C_CrouchReq& pkt);
   bool Handle_C_WireActionReq(PacketSessionRef& session, const se::game::C_WireActionReq& pkt);
   bool Handle_C_WireActionEnd(PacketSessionRef& session, const se::game::C_WireActionEnd& pkt);
   bool Handle_C_WireLaunchReq(PacketSessionRef& session, const se::game::C_WireLaunchReq& pkt);
   bool Handle_C_AimReq(PacketSessionRef& session, const se::game::C_AimReq& pkt);
   bool Handle_C_FireReq(PacketSessionRef& session, const se::game::C_FireReq& pkt);
   bool Handle_C_ThrowGrenadeReq(PacketSessionRef& session, const se::game::C_ThrowGrenadeReq& pkt);
   bool Handle_C_GrenadeMoveSyncReq(PacketSessionRef& session, const se::game::C_GrenadeMoveSyncReq& pkt);
   bool Handle_C_GrenadeExplosionReq(PacketSessionRef& session, const se::game::C_GrenadeExplosionReq& pkt);
   bool Handle_C_ReloadReq(PacketSessionRef& session, const se::game::C_ReloadReq& pkt);
   bool Handle_C_WeaponChangeReq(PacketSessionRef& session, const se::game::C_WeaponChangeReq& pkt);
   bool Handle_C_UseSkillReq(PacketSessionRef& session, const se::game::C_UseSkillReq& pkt);
   bool Handle_C_UseItemReq(PacketSessionRef& session, const se::game::C_UseItemReq& pkt);
   bool Handle_C_ChestInteractReq(PacketSessionRef& session, const se::game::C_ChestInteractReq& pkt);
   bool Handle_C_PickupItemReq(PacketSessionRef& session, const se::game::C_PickupItemReq& pkt);
   bool Handle_C_UseStoreReq(PacketSessionRef& session, const se::game::C_UseStoreReq& pkt);
   bool Handle_C_SetSavePointReq(PacketSessionRef& session, const se::game::C_SetSavePointReq& pkt);
   bool Handle_C_EquipItemReq(PacketSessionRef& session, const se::game::C_EquipItemReq& pkt);
   bool Handle_C_SkillEquipReq(PacketSessionRef& session, const se::game::C_SkillEquipReq& pkt);
   
// Test Packet
public:
   bool Handle_C_SpawnMonsterReq(PacketSessionRef& session, const se::test::C_SpawnMonsterReq& pkt);
   bool Handle_C_SpawnChestReq(PacketSessionRef& session, const se::test::C_SpawnChestReq& pkt);
   bool Handle_C_SpawnStoreReq(PacketSessionRef& session, const se::test::C_SpawnStoreReq& pkt);
   bool Handle_C_ItemReq(PacketSessionRef& session, const se::test::C_ItemReq& pkt);
   bool Handle_C_ItemReqAll(PacketSessionRef& session, const se::test::C_ItemReqAll& pkt);
   bool Handle_C_MoneyReq(PacketSessionRef& session, const se::test::C_MoneyReq& pkt);
   bool Handle_C_HealthReq(PacketSessionRef& session, const se::test::C_HealthReq& pkt);
   bool Handle_C_MaxHealthReq(PacketSessionRef& session, const se::test::C_MaxHealthReq& pkt);
   bool Handle_C_MoneyReqAll(PacketSessionRef& session, const se::test::C_MoneyReqAll& pkt);
   bool Handle_C_HealthReqAll(PacketSessionRef& session, const se::test::C_HealthReqAll& pkt);
   bool Handle_C_MaxHealthReqAll(PacketSessionRef& session, const se::test::C_MaxHealthReqAll& pkt);
   bool Handle_C_TPAllReq(PacketSessionRef& session, const se::test::C_TPAllReq& pkt);
   bool Handle_C_ZoneStopReq(PacketSessionRef& session, const se::test::C_ZoneStopReq& pkt);
   bool Handle_C_ZoneStartReq(PacketSessionRef& session, const se::test::C_ZoneStartReq& pkt);
   bool Handle_C_ZoneResetReq(PacketSessionRef& session, const se::test::C_ZoneResetReq& pkt);
   bool Handle_C_ZoneDamageOffReq(PacketSessionRef& session, const se::test::C_ZoneDamageOffReq& pkt);
   bool Handle_C_ZoneDamageOnReq(PacketSessionRef& session, const se::test::C_ZoneDamageOnReq& pkt);
   
private:
   bool TryGetPlayerId(PacketSessionRef& session, PlayerId& outPlayerId) const;
   bool TryResolvePlayerRoute(PlayerId playerId, PlayerRoute& outRoute) const;
   Room* FindPlayerRoom(PlayerId playerId) const;
   
private:
   template <typename Packet, typename Fn>
   bool EnqueueToPlayerRoom(PacketSessionRef& session, const Packet& pkt, Fn&& fn)
   {
      PlayerId playerId = 0;
      if (!TryGetPlayerId(session, playerId)) return false;
      
      PlayerRoute route;
      if (!TryResolvePlayerRoute(playerId, route)) return false;
      
      auto* shardManager = shardManager_;
      if (!shardManager) return false;
      
      return shardManager->Enqueue(route.shardId, [shardManager, route, pkt, fn = std::forward<Fn>(fn)]() mutable
      {
         auto* shard = shardManager->GetShard(route.shardId);
         if (!shard) return;
         
         auto room = shard->FindRoom(route.roomId);
         if (!room) return;
         
         fn(*room, route.playerId, pkt);
      });
      
   }
   
private:
   SessionManager& sessionManager_;
   PlayerManager& playerManager_;
   MatchMaker& matchMaker_;
   RoomDirectory& roomDirectory_;
   ShardManager* shardManager_ = nullptr;
    
};
