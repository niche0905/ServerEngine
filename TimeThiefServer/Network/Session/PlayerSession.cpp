#include "pch.h"
#include "PlayerSession.h"

#include "SessionManager/SessionManager.h"
#include "SessionIdMaker.h"
#include "Content/Player/PlayerManager/PlayerManager.h"
#include "Content/Room/Room.h"
#include "Generated/ServerPacketHandler.h"
#include "Protocol/ProtocolVersion.h"

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
   auto ioObject = shared_from_this();
   auto session = std::static_pointer_cast<PacketSession>(ioObject);
   
   ServerPacketHandler::Dispatch(session, buffer, len);
}

bool PlayerSession::HandleHandshake(const se::auth::C_HandshakeReq& pkt)
{
   if (state_ != PlayerSessionState::Handshaking) 
      return false;
   
   if (pkt.client_protocol_version() != se::protocol::kProtocolVersion) {
      SendHandshakeRes(false, se::common::ERR_INVALID_PROTOCOL_VERSION, "Protocol version mismatch");
      Disconnect(L"Incompatible protocol version");
      return false;
   }
   
   SendHandshakeRes(true, se::common::ERR_NONE, "OK");
   
   state_ = PlayerSessionState::InLobby;
   return true;
}

void PlayerSession::SendHandshakeRes(bool success, se::common::ErrorCode errorCode, const std::string& errorMessage)
{
   se::auth::S_HandshakeRes handshakeRes;
   handshakeRes.set_success(success);
   auto* result = handshakeRes.mutable_result();
   result->set_code(errorCode);
   result->set_message(errorMessage);
   
   PlayerId playerId = 0;
   if (g_SessionManager.TryGetPlayerId(Id(), playerId)) {
      handshakeRes.set_session_player_id(playerId);
   }
   auto* config = handshakeRes.mutable_config();
   config->set_movement_update_hz(10);   // TODO: 이 값은 .ini나 .config 파일로 부터 읽어와서 적용해야 할 듯 싶다
   config->set_ping_interval_ms(1000);   // TODO: 이 값은 .ini나 .config 파일로 부터 읽어와서 적용해야 할 듯 싶다
   
   auto buffer = ServerPacketHandler::MakeSendBuffer(handshakeRes);
   Send(buffer);
}


void PlayerSession::OnConnected()
{
   SessionId newSessionId = SessionIdMaker::Next();
   AssignId(newSessionId);
   g_SessionManager.Add(newSessionId, AsShared<PlayerSession>());
   g_SessionManager.BindPlayer(newSessionId, newSessionId);    // TEMP
   
   state_ = PlayerSessionState::Handshaking;
}

void PlayerSession::OnDisconnected()
{
   PlayerId playerId = 0;
   bool binding = g_SessionManager.TryGetPlayerId(Id(), playerId);
   if (binding) {
      // TODO: 플레이어가 방에 입장해 있는 상태라면 방에서도 제거하기 (방 정보는 Player 객체에 캐싱되어 있으므로 PlayerManager에서 제거할 때 RoomManager에도 알려주는 방식으로 구현할 수 있을 듯)
      auto playerRef = g_PlayerManager.Find(playerId);
      if (playerRef) {
         RoomId roomId = playerRef->roomId_;
         // TODO: RoomManager에서 방 정보를 찾아서 플레이어 제거 요청하기 (방 정보는 Player 객체에 캐싱되어 있으므로 PlayerManager에서 제거할 때 RoomManager에도 알려주는 방식으로 구현할 수 있을 듯)
         auto roomRef = GRoom;   // TEMP
         if (roomRef) {
            roomRef->Leave(playerId);
         }
      }
      
      // TODO: 지금은 Player 정보를 아예 지워 버리지만, 재 로그인을 예상하여 SessionId만 invalid 처리하는 것도 방법일듯 싶다
      g_PlayerManager.Remove(playerId);
   }
   
   g_SessionManager.RemoveBySessionId(Id());
}

void PlayerSession::OnSend(int32 len)
{
}
