#include "pch.h"
#include "PlayerSession.h"

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
   wprintf(L"[PlayerSession] Received Packet: %S\n", message.c_str());
}

void PlayerSession::OnConnected()
{
   // TODO: Session 매니저를 생성해서 관리해야 한다 (Backend 코어가 아닌 게임컨텐츠(서비스)에서 사용할 용도)
}

void PlayerSession::OnDisconnected()
{
   // TODO: Session 매니저에서 제거 처리
}

void PlayerSession::OnSend(int32 len)
{
   // nothing to do now
}
