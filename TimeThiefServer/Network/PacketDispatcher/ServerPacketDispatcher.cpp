#include "pch.h"
#include "ServerPacketDispatcher.h"
#include "Network/Session/PacketSession.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Service/Player/PlayerManager/PlayerManager.h"
#include "Service/Room/RoomManager.h"
#include "Service/Room/Room.h"
#include "Routing/PlayerRoute.h"
#include "Service/MatchMaking/MatchMaker.h"
#include "Shard/RoomDirectory.h"
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
    // Session 레벨에서 진행하기에 이 함수가 불릴 일은 없다
    return false;
}

bool ServerPacketDispatcher::Handle_C_LoginReq(PacketSessionRef& session, const se::auth::C_LoginReq& pkt)
{
    return false;
}

bool ServerPacketDispatcher::Handle_C_Ping(PacketSessionRef& session, const se::auth::C_Ping& pkt)
{
    // 바로 Pong 응답 보내기에 이 함수가 불릴 일은 없다
    return false;
}

bool ServerPacketDispatcher::Handle_C_SetNicknameReq(PacketSessionRef& session, const se::lobby::C_SetNicknameReq& pkt)
{
    PlayerId playerId = 0;
    if (!sessionManager_.TryGetPlayerId(session->Id(), playerId)) return false;
    
    if (playerId == 0) return false;
    
    se::lobby::S_SetNicknameRes resPkt;
    auto player = playerManager_.Find(playerId);
    if (!player) {
        auto* resultPtr = resPkt.mutable_result();
        resultPtr->set_code(se::common::ERR_INTERNAL_ERROR);
        resultPtr->set_message("Player not found for the session");
    }
    
    bool success = player->TrySetNickname(pkt.nickname());
    resPkt.set_success(success);
    if (success) {
        resPkt.set_nickname(pkt.nickname());
    }
    else {
        auto* resultPtr = resPkt.mutable_result();
        resultPtr->set_code(se::common::ERR_NICKNAME_INVALID);
        resultPtr->set_message("Failed to set nickname");
    }
    
    auto sendBuffer = ServerPacketHandler::MakeSendBuffer(resPkt);
    if (!sendBuffer) return false;
    
    session->Send(sendBuffer);
    return true;
}

bool ServerPacketDispatcher::Handle_C_MatchQueueEnterReq(PacketSessionRef& session,
    const se::lobby::C_MatchQueueEnterReq& pkt)
{
    PlayerId playerId = 0;
    if (!sessionManager_.TryGetPlayerId(session->Id(), playerId)) return false;
    
    if (playerId == 0) return false;
    
    bool succ = matchMaker_.Enqueue(playerId);
    se::lobby::S_MatchQueueEnterRes resPkt;
    resPkt.set_success(succ);
    if (!succ) {
        auto* resultPtr = resPkt.mutable_result();
        resultPtr->set_code(se::common::ERR_MATCHMAKING_UNAVAILABLE);
        resultPtr->set_message("Failed to enter matchmaking queue");
    }
    
    auto sendBuffer = ServerPacketHandler::MakeSendBuffer(resPkt);
    if (!sendBuffer) return false;
    
    session->Send(sendBuffer);
    return true;
}

bool ServerPacketDispatcher::Handle_C_MatchQueueCancelReq(PacketSessionRef& session,
    const se::lobby::C_MatchQueueCancelReq& pkt)
{
    PlayerId playerId = 0;
    if (!sessionManager_.TryGetPlayerId(session->Id(), playerId)) return false;
    
    if (playerId == 0) return false;
    
    bool succ = matchMaker_.Cancel(playerId);
    se::lobby::S_MatchQueueCancelRes resPkt;
    resPkt.set_success(succ);
    if (!succ) {
        auto* resultPtr = resPkt.mutable_result();
        resultPtr->set_code(se::common::ERR_NOT_IN_MATCH_QUEUE);
        resultPtr->set_message("Failed to cancel matchmaking queue");
    }
    
    auto sendBuffer = ServerPacketHandler::MakeSendBuffer(resPkt);
    if (!sendBuffer) return false;
    
    session->Send(sendBuffer);
    return true;
}

bool ServerPacketDispatcher::Handle_C_RoomEnterReq(PacketSessionRef& session, const se::room::C_RoomEnterReq& pkt)
{
    SessionId sessionId = session->Id();
    
    PlayerId playerId = 0;
    if (!TryGetPlayerId(session, playerId)) return false;
      
    auto* shardManager = shardManager_;
    if (!shardManager) return false;
    
    auto* playerManager = &playerManager_;
    if (!playerManager) return false;
    
    RoomId clientRoomId = pkt.room_id();
    if (clientRoomId == 0) return false;
    
    // TODO: RoomDirectory에서 RoomId로 ShardId, RoomId 찾는 구조로 변경하기 (현재는 RoomDirectory가 ShardManager보다 아래에 있어서 ShardManager에서 RoomDirectory를 참조하는 구조로 되어 있지만, RoomDirectory가 ShardManager보다 위에 있는 구조로 변경하기)
    auto shardCandi = roomDirectory_.FindShardId(clientRoomId);
    if (!shardCandi) return false;
    ShardId shardId = *shardCandi;
    
    PlayerRoute route = {.playerId = playerId, .roomId = clientRoomId, .shardId = shardId};
    if (!route.IsValid()) return false;
    
    return shardManager->Enqueue(route.shardId, [shardManager, playerManager, route, sessionId]()
    {
        auto* shard = shardManager->GetShard(route.shardId);
        if (!shard) return;
        
        auto room = shard->FindRoom(route.roomId);
        if (!room) return;
        
        if (!room->Join(route.playerId, sessionId))
            return;
        
        playerManager->UpdateRoute(route.playerId, route.shardId, route.roomId);
    });
}

bool ServerPacketDispatcher::Handle_C_RoomLeaveReq(PacketSessionRef& session, const se::room::C_RoomLeaveReq& pkt)
{
    PlayerId playerId = 0;
    if (!TryGetPlayerId(session, playerId)) return false;
      
    PlayerRoute route;
    if (!TryResolvePlayerRoute(playerId, route)) return false;
      
    auto* shardManager = shardManager_;
    if (!shardManager) return false;
    
    return shardManager->Enqueue(route.shardId, [shardManager, route]()
    {
        auto* shard = shardManager->GetShard(route.shardId);
        if (!shard) return;
        
        auto room = shard->FindRoom(route.roomId);
        if (!room) return;
        
        room->Leave(route.playerId);
    });
}

bool ServerPacketDispatcher::Handle_C_LoadingCompleteReq(PacketSessionRef& session,
    const se::game::C_LoadingCompleteReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_LoadingCompleteReq& pkt)
    {
        room.HandleLoadingComplete(playerId);
    });
}

bool ServerPacketDispatcher::Handle_C_MoveReq(PacketSessionRef& session, const se::game::C_MoveReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_MoveReq& pkt)
    {
        room.HandleMove(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_JumpReq(PacketSessionRef& session, const se::game::C_JumpReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_JumpReq& pkt)
    {
        room.HandleJump(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_JumpLand(PacketSessionRef& session, const se::game::C_JumpLand& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_JumpLand& pkt)
    {
        room.HandleJumpLand(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_DoubleJumpReq(PacketSessionRef& session, const se::game::C_DoubleJumpReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_DoubleJumpReq& pkt)
    {
        room.HandleDoubleJump(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_CrouchReq(PacketSessionRef& session, const se::game::C_CrouchReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_CrouchReq& pkt)
    {
        room.HandleCrouch(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_WireActionReq(PacketSessionRef& session, const se::game::C_WireActionReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_WireActionReq& pkt)
    {
        room.HandleWireAction(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_WireActionEnd(PacketSessionRef& session, const se::game::C_WireActionEnd& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_WireActionEnd& pkt)
    {
        room.HandleWireActionEnd(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_WireLaunchReq(PacketSessionRef& session, const se::game::C_WireLaunchReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_WireLaunchReq& pkt)
    {
        room.HandleWireLaunch(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_AimReq(PacketSessionRef& session, const se::game::C_AimReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_AimReq& pkt)
    {
        room.HandleAim(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_FireReq(PacketSessionRef& session, const se::game::C_FireReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_FireReq& pkt)
    {
        room.HandleFire(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_ThrowGrenadeReq(PacketSessionRef& session, const se::game::C_ThrowGrenadeReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_ThrowGrenadeReq& pkt)
    {
        room.HandleThrowGrenade(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_ReloadReq(PacketSessionRef& session, const se::game::C_ReloadReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_ReloadReq& pkt)
    {
        room.HandleReload(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_WeaponChangeReq(PacketSessionRef& session, const se::game::C_WeaponChangeReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_WeaponChangeReq& pkt)
    {
        room.HandleWeaponChange(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_UseAbilityReq(PacketSessionRef& session, const se::game::C_UseAbilityReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_UseAbilityReq& pkt)
    {
        room.HandleUseAbility(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_UseItemReq(PacketSessionRef& session, const se::game::C_UseItemReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_UseItemReq& pkt)
    {
        room.HandleUseItem(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_ChestInteractReq(PacketSessionRef& session,
    const se::game::C_ChestInteractReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_ChestInteractReq& pkt)
    {
        room.HandleChestInteract(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_PickupItemReq(PacketSessionRef& session, const se::game::C_PickupItemReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_PickupItemReq& pkt)
    {
        room.HandlePickupItem(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_UseStoreReq(PacketSessionRef& session, const se::game::C_UseStoreReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_UseStoreReq& pkt)
    {
        room.HandleUseStore(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_SetSavePointReq(PacketSessionRef& session, const se::game::C_SetSavePointReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_SetSavePointReq& pkt)
    {
        room.HandleSetSavePoint(playerId, pkt);
    });
}

bool ServerPacketDispatcher::Handle_C_EquipItemReq(PacketSessionRef& session, const se::game::C_EquipItemReq& pkt)
{
    return EnqueueToPlayerRoom(session, pkt, [](Room& room, PlayerId playerId, const se::game::C_EquipItemReq& pkt)
    {
        room.HandleEquipItem(playerId, pkt);
    });
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
    
    if (player->shardId_ == 0 or player->roomId_ == 0) return nullptr;
    
    return shardManager_->GetShard(player->shardId_)->FindRoom(player->roomId_).get();
}
