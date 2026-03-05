#include "pch.h"
#include "PlayerSession.h"

#include "SessionManager/SessionManager.h"
#include "SessionIdMaker.h"
#include "Content/Player/PlayerManager/PlayerManager.h"

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
   SessionId newSessionId = SessionIdMaker::Next();
   AssignId(newSessionId);
   g_SessionManager.Add(newSessionId, AsShared<PlayerSession>());
}

void PlayerSession::OnDisconnected()
{
   PlayerId playerId = 0;
   bool binding = g_SessionManager.TryGetPlayerId(Id(), playerId);
   if (binding) {
      // TODO: 지금은 Player 정보를 아예 지워 버리지만, 재 로그인을 예상하여 SessionId만 invalid 처리하는 것도 방법일듯 싶다
      g_PlayerManager.Remove(playerId);
      
      // TODO: 플레이어가 방에 입장해 있는 상태라면 방에서도 제거하기 (방 정보는 Player 객체에 캐싱되어 있으므로 PlayerManager에서 제거할 때 RoomManager에도 알려주는 방식으로 구현할 수 있을 듯)
   }
   
   g_SessionManager.RemoveBySessionId(Id());
}

void PlayerSession::OnSend(int32 len)
{
   // nothing to do now
}
