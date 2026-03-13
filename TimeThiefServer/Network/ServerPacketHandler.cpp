#include "pch.h"
#include "Generated/ServerPacketHandler.h"

#include "Content/Room/Room.h"
#include "Session/PlayerSession.h"

PacketHandlerFunc GPacketHandler[kMaxMessageId + 1];

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

bool Handle_C_LobbyEnterReq(PacketSessionRef& session, const se::lobby::C_LobbyEnterReq& pkt)
{
    // TEMP: 임시로 PlayerId와 SessionId 같게..
    return GRoom->Join(session->Id(), session->Id());
}

bool Handle_C_MatchQueueEnterReq(PacketSessionRef& session, const se::lobby::C_MatchQueueEnterReq& pkt)
{
    return false;
}

bool Handle_C_MatchQueueCancelReq(PacketSessionRef& session, const se::lobby::C_MatchQueueCancelReq& pkt)
{
    return false;
}

bool Handle_C_MoveInput(PacketSessionRef& session, const se::room::C_MoveInput& pkt)
{
    return GRoom->HandleMove(session->Id(), pkt);
}

bool Handle_C_AimInput(PacketSessionRef& session, const se::room::C_AimInput& pkt)
{
    return false;
}

bool Handle_C_FireReq(PacketSessionRef& session, const se::room::C_FireReq& pkt)
{
    return false;
}
