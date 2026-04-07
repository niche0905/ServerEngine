#include "pch.h"
#include "PlayerSession.h"

#include "SessionManager/SessionManager.h"
#include "SessionIdMaker.h"
#include "Service/Player/PlayerManager/PlayerManager.h"
#include "Service/Room/Room.h"
#include "Service/Room/RoomManager.h"
#include "Generated/ServerPacketHandler.h"
#include "Lifecycle/IPlayerSessionLifecycle.h"
#include "Protocol/ProtocolVersion.h"

/*-----------------
   PlayerSession
-----------------*/

PlayerSession::PlayerSession(IPlayerSessionLifecycle& lifecycle)
   : lifecycle_(lifecycle)
{
}

PlayerSession::~PlayerSession() = default;

void PlayerSession::Dispatch(class IIoEvent* ioEvent, int32 numOfBytes)
{
   switch (ioEvent->GetType())
   {
      case IoEventType::Connect:
         ProcessConnect();
         break;
      case IoEventType::Disconnect:
         ProcessDisconnect();
         break;
      case IoEventType::Recv:
         ProcessRecv(numOfBytes);
         break;
      case IoEventType::Send:
         ProcessSend(numOfBytes);
         break;
      default:
         assert(false && "PlayerSession::Dispatch - Unknown IoEventType");
         break;
   }
}

bool PlayerSession::CanPacketProcess(const byte* buffer, int32 len)
{
   if (len < GetPacketHeaderSize())
      return false;
   
   int32 packetSize = GetPacketSize(const_cast<byte*>(buffer));
   return (len >= packetSize);
}

void PlayerSession::OnRecvPacket(byte* buffer, int32 len)
{
   auto ioObject = shared_from_this();
   auto session = std::static_pointer_cast<PacketSession>(ioObject);
   
   ServerPacketHandler::Dispatch(session, buffer, len);
}

bool PlayerSession::HandleHandshake(const se::auth::C_HandshakeReq& pkt)
{
   return lifecycle_.HandleHandshake(*this, pkt);
}

void PlayerSession::OnConnected()
{
   lifecycle_.OnConnected(*this);
}

void PlayerSession::OnDisconnected()
{
   lifecycle_.OnDisconnected(*this);
}

void PlayerSession::OnSend(int32 len)
{
}
