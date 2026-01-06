#include "pch.h"
#include "PlayerSession.h"

#include "SessionManager/SessionManager.h"

/*-----------------
   PlayerSession
-----------------*/

PlayerSession::PlayerSession() = default;

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
   // TEMP: 간단한 문자열 통신 테스트
   std::string message(reinterpret_cast<char*>(buffer) + 1, len - 1);
   consoleLogger->Log(Color::Blue, L"[PlayerSession] Received Packet: %S\n", message.c_str());
}

void PlayerSession::OnConnected()
{
   g_SessionManager.Add(AsShared<PlayerSession>());
}

void PlayerSession::OnDisconnected()
{
   g_SessionManager.Remove(AsShared<PlayerSession>());
}

void PlayerSession::OnSend(int32 len)
{
   // nothing to do now
}
