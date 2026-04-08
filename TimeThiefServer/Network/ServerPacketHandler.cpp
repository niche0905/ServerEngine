#include "pch.h"
#include "Generated/ServerPacketHandler.h"

#include "PacketDispatcher/ServerPacketDispatcher.h"
#include "Session/PlayerSession.h"

PacketHandlerFunc GPacketHandler[kMaxMessageId + 1];

namespace 
{
    ServerPacketDispatcher* GServerPacketDispatcher = nullptr;  // non-owning
    
    inline ServerPacketDispatcher* Dispatcher()
    {
        return GServerPacketDispatcher;
    }
}

void SetServerPacketDispatcher(ServerPacketDispatcher* dispatcher)
{
    GServerPacketDispatcher = dispatcher;
}

ServerPacketDispatcher* GetServerPacketDispatcher()
{
    return GServerPacketDispatcher;
}

bool Handle_INVALID(PacketSessionRef& session, BYTE* buffer, int32 len)
{
    return false;
}


bool Handle_C_HandshakeReq(PacketSessionRef& session, const se::auth::C_HandshakeReq& pkt)
{
    if (!session) return false;
    
    auto playerSession = std::static_pointer_cast<PlayerSession>(session);
    if (!playerSession) return false;
    
    return playerSession->HandleHandshake(pkt);
}
    
bool Handle_C_LoginReq(PacketSessionRef& session, const se::auth::C_LoginReq& pkt)
{
    return false;
}
    
bool Handle_C_Ping(PacketSessionRef& session, const se::auth::C_Ping& pkt)
{
    if (!session) return false;
    
    // THINK: Session에서 Ping/Pong을 가지고 timeout 판정을 하거나, 개별로 RTT를 필요로 한다면 HandlePing 멤버함수 만들기
    //        지금은 간단한 Pong 응답만 보내도록 구현
    
    se::auth::S_Pong pongPkt;
    pongPkt.set_client_time_ms(pkt.client_time_ms());
    // pongPkt.set_server_time_ms(GetCurrentTimeMs());  // 아직 서버 시간 보내는 기능이 필요하지 않아서 주석 처리, 필요해지면 구현 예정
    
    auto sendBuffer = ServerPacketHandler::MakeSendBuffer(pongPkt);
    if (!sendBuffer) return false;
    
    session->Send(sendBuffer);
    return true;
}
    
bool Handle_C_SetNicknameReq(PacketSessionRef& session, const se::lobby::C_SetNicknameReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_SetNicknameReq(session, pkt);
}
    
bool Handle_C_MatchQueueEnterReq(PacketSessionRef& session, const se::lobby::C_MatchQueueEnterReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_MatchQueueEnterReq(session, pkt);
}
    
bool Handle_C_MatchQueueCancelReq(PacketSessionRef& session, const se::lobby::C_MatchQueueCancelReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_MatchQueueCancelReq(session, pkt);
}
    
bool Handle_C_RoomEnterReq(PacketSessionRef& session, const se::room::C_RoomEnterReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_RoomEnterReq(session, pkt);
}
    
bool Handle_C_RoomLeaveReq(PacketSessionRef& session, const se::room::C_RoomLeaveReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_RoomLeaveReq(session, pkt);
}
    
bool Handle_C_LoadingCompleteReq(PacketSessionRef& session, const se::game::C_LoadingCompleteReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_LoadingCompleteReq(session, pkt);
}
    
bool Handle_C_MoveReq(PacketSessionRef& session, const se::game::C_MoveReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_MoveReq(session, pkt);
}

bool Handle_C_JumpReq(PacketSessionRef& session, const se::game::C_JumpReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_JumpReq(session, pkt);
}

bool Handle_C_JumpLand(PacketSessionRef& session, const se::game::C_JumpLand& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_JumpLand(session, pkt);
}
    
bool Handle_C_CrouchReq(PacketSessionRef& session, const se::game::C_CrouchReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_CrouchReq(session, pkt);
}
    
bool Handle_C_WireActionReq(PacketSessionRef& session, const se::game::C_WireActionReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_WireActionReq(session, pkt);
}
    
bool Handle_C_WireActionEnd(PacketSessionRef& session, const se::game::C_WireActionEnd& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_WireActionEnd(session, pkt);
}
    
bool Handle_C_AimReq(PacketSessionRef& session, const se::game::C_AimReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_AimReq(session, pkt);
}
    
bool Handle_C_FireReq(PacketSessionRef& session, const se::game::C_FireReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_FireReq(session, pkt);
}
    
bool Handle_C_AttackReq(PacketSessionRef& session, const se::game::C_AttackReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_AttackReq(session, pkt);
}
    
bool Handle_C_ThrowGrenadeReq(PacketSessionRef& session, const se::game::C_ThrowGrenadeReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_ThrowGrenadeReq(session, pkt);
}
    
bool Handle_C_ReloadReq(PacketSessionRef& session, const se::game::C_ReloadReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_ReloadReq(session, pkt);
}
    
bool Handle_C_WeaponChangeReq(PacketSessionRef& session, const se::game::C_WeaponChangeReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_WeaponChangeReq(session, pkt);
}
    
bool Handle_C_UseAbilityReq(PacketSessionRef& session, const se::game::C_UseAbilityReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_UseAbilityReq(session, pkt);
}
    
bool Handle_C_UseItemReq(PacketSessionRef& session, const se::game::C_UseItemReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_UseItemReq(session, pkt);
}
    
bool Handle_C_ChestInteractReq(PacketSessionRef& session, const se::game::C_ChestInteractReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_ChestInteractReq(session, pkt);
}
    
bool Handle_C_PickupItemReq(PacketSessionRef& session, const se::game::C_PickupItemReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_PickupItemReq(session, pkt);
}
    
bool Handle_C_UseStoreReq(PacketSessionRef& session, const se::game::C_UseStoreReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_UseStoreReq(session, pkt);
}
    
bool Handle_C_SetSavePointReq(PacketSessionRef& session, const se::game::C_SetSavePointReq& pkt)
{
    if (!Dispatcher()) return false;
    return Dispatcher()->Handle_C_SetSavePointReq(session, pkt);
}
    
