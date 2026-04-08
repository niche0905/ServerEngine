#include "pch.h"
#include "ServerPacketDispatcher.h"
#include "Network/Session/PacketSession.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Service/Player/PlayerManager/PlayerManager.h"
#include "Service/Room/RoomManager.h"
#include "Service/Room/Room.h"
#include "Routing/PlayerRoute.h"
#include "Shard/ShardManager.h"

ServerPacketDispatcher::ServerPacketDispatcher(SessionManager& sessionManager, PlayerManager& playerManager,
                                               MatchMaker& matchMaker, RoomDirectory& roomDirectory, ShardManager* shardManager)
        : sessionManager_(sessionManager)
          , playerManager_(playerManager)
          , matchMaker_(matchMaker)
          , roomDirectory_(roomDirectory)
          , shardManager_(shardManager)
{
}

bool ServerPacketDispatcher::Handle_C_HandshakeReq(PacketSessionRef& session, const se::auth::C_HandshakeReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_LoginReq(PacketSessionRef& session, const se::auth::C_LoginReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_Ping(PacketSessionRef& session, const se::auth::C_Ping& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_SetNicknameReq(PacketSessionRef& session, const se::lobby::C_SetNicknameReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_MatchQueueEnterReq(PacketSessionRef& session,
    const se::lobby::C_MatchQueueEnterReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_MatchQueueCancelReq(PacketSessionRef& session,
    const se::lobby::C_MatchQueueCancelReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_RoomEnterReq(PacketSessionRef& session, const se::room::C_RoomEnterReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_RoomLeaveReq(PacketSessionRef& session, const se::room::C_RoomLeaveReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_LoadingCompleteReq(PacketSessionRef& session,
    const se::game::C_LoadingCompleteReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_MoveReq(PacketSessionRef& session, const se::game::C_MoveReq& pkt)
{
    PlayerId playerId = 0;
    if (!TryGetPlayerId(session, playerId)) return false;
    
    PlayerRoute route;
    if (!TryResolvePlayerRoute(playerId, route)) return false;
    
    auto* shardManager = shardManager_;
    if (!shardManager) return false;
    
    return shardManager->Enqueue(route.shardId, [shardManager, route, pkt]()
    {
        auto shard = shardManager->GetShard(route.shardId);
        if (!shard) return;
        
        auto room = shard->FindRoom(route.roomId);
        if (!room) return;
        
        room->HandleMove(route.playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_JumpReq(PacketSessionRef& session, const se::game::C_JumpReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_JumpLand(PacketSessionRef& session, const se::game::C_JumpLand& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_CrouchReq(PacketSessionRef& session, const se::game::C_CrouchReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_WireActionReq(PacketSessionRef& session, const se::game::C_WireActionReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_WireActionEnd(PacketSessionRef& session, const se::game::C_WireActionEnd& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_AimReq(PacketSessionRef& session, const se::game::C_AimReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_FireReq(PacketSessionRef& session, const se::game::C_FireReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_AttackReq(PacketSessionRef& session, const se::game::C_AttackReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_ThrowGrenadeReq(PacketSessionRef& session, const se::game::C_ThrowGrenadeReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_ReloadReq(PacketSessionRef& session, const se::game::C_ReloadReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_WeaponChangeReq(PacketSessionRef& session, const se::game::C_WeaponChangeReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_UseAbilityReq(PacketSessionRef& session, const se::game::C_UseAbilityReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_UseItemReq(PacketSessionRef& session, const se::game::C_UseItemReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_ChestInteractReq(PacketSessionRef& session,
    const se::game::C_ChestInteractReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_PickupItemReq(PacketSessionRef& session, const se::game::C_PickupItemReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_UseStoreReq(PacketSessionRef& session, const se::game::C_UseStoreReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_SetSavePointReq(PacketSessionRef& session, const se::game::C_SetSavePointReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::TryGetPlayerId(PacketSessionRef& session, PlayerId& outPlayerId) const
{
    outPlayerId = 0;
    
    if (!session) return false;
    
    SessionId sessionId = session->Id();
    if (sessionId == 0) return false;
    
    return sessionManager_.TryGetPlayerId(sessionId, outPlayerId) and outPlayerId != 0;
}

bool ServerPacketDispatcher::TryResolvePlayerRoute(PlayerId playerId, PlayerRoute& outRoute) const
{
    outRoute = {};
    
    if (playerId == 0) return false;
    
    auto player = playerManager_.Find(playerId);
    if (!player) return false;
    
    // TODO: shard id가 0인 경우는 고민이 좀 더 필요
    if (player->roomId_ == 0 or player->shardId_ == 0) return false;
    
    outRoute.playerId = playerId;
    outRoute.shardId = player->shardId_;
    outRoute.roomId = player->roomId_;
    return true;
}

Room* ServerPacketDispatcher::FindPlayerRoom(PlayerId playerId) const
// 이 함수를 쓸 땐 현재 thread의 ShardId와 해당 player의 shardId가 일치하는지 확인해야 한다 (Shard 정책에 의해)
{
    if (playerId == 0) return nullptr;
    
    auto player = playerManager_.Find(playerId);
    if (!player) return nullptr;
    
    // if (player->shardId_ == 0 or player->roomId_ == 0) return nullptr;
    
    return shardManager_->GetShard(player->shardId_)->FindRoom(player->roomId_).get();
}
