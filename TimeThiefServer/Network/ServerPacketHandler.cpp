#include "pch.h"
#include "Generated/ServerPacketHandler.h"

#include "Content/Room/Room.h"
#include "Session/PlayerSession.h"

PacketHandlerFunc GPacketHandler[kMaxMessageId + 1];


bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    return false;
}


bool Handle_C_HandshakeReq(PacketSessionRef& session, const se::auth::C_HandshakeReq& pkt)
{
    return false;
}
    
bool Handle_C_LoginReq(PacketSessionRef& session, const se::auth::C_LoginReq& pkt)
{
    return false;
}
    
bool Handle_C_Ping(PacketSessionRef& session, const se::auth::C_Ping& pkt)
{
    return false;
}
    
bool Handle_C_SetNicknameReq(PacketSessionRef& session, const se::lobby::C_SetNicknameReq& pkt)
{
    return false;
}
    
bool Handle_C_MatchQueueEnterReq(PacketSessionRef& session, const se::lobby::C_MatchQueueEnterReq& pkt)
{
    return false;
}
    
bool Handle_C_MatchQueueCancelReq(PacketSessionRef& session, const se::lobby::C_MatchQueueCancelReq& pkt)
{
    return false;
}
    
bool Handle_C_RoomEnterReq(PacketSessionRef& session, const se::room::C_RoomEnterReq& pkt)
{
    return false;
}
    
bool Handle_C_RoomLeaveReq(PacketSessionRef& session, const se::room::C_RoomLeaveReq& pkt)
{
    return false;
}
    
bool Handle_C_LoadingCompleteReq(PacketSessionRef& session, const se::game::C_LoadingCompleteReq& pkt)
{
    return false;
}
    
bool Handle_C_MoveReq(PacketSessionRef& session, const se::game::C_MoveReq& pkt)
{
    return false;
}
    
bool Handle_C_FireReq(PacketSessionRef& session, const se::game::C_FireReq& pkt)
{
    return false;
}
    
bool Handle_C_AttackReq(PacketSessionRef& session, const se::game::C_AttackReq& pkt)
{
    return false;
}
    
bool Handle_C_ThrowGrenadeReq(PacketSessionRef& session, const se::game::C_ThrowGrenadeReq& pkt)
{
    return false;
}
    
bool Handle_C_ReloadReq(PacketSessionRef& session, const se::game::C_ReloadReq& pkt)
{
    return false;
}
    
bool Handle_C_WeaponChangeReq(PacketSessionRef& session, const se::game::C_WeaponChangeReq& pkt)
{
    return false;
}
    
bool Handle_C_UseAbilityReq(PacketSessionRef& session, const se::game::C_UseAbilityReq& pkt)
{
    return false;
}
    
bool Handle_C_UseItemReq(PacketSessionRef& session, const se::game::C_UseItemReq& pkt)
{
    return false;
}
    
bool Handle_C_ChestInteractReq(PacketSessionRef& session, const se::game::C_ChestInteractReq& pkt)
{
    return false;
}
    
bool Handle_C_PickupItemReq(PacketSessionRef& session, const se::game::C_PickupItemReq& pkt)
{
    return false;
}
    
bool Handle_C_UseStoreReq(PacketSessionRef& session, const se::game::C_UseStoreReq& pkt)
{
    return false;
}
    
bool Handle_C_SetSavePointReq(PacketSessionRef& session, const se::game::C_SetSavePointReq& pkt)
{
    return false;
}
    
