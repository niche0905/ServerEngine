#include "pch.h"
#include "PlayerSessionLifecycleService.h"

#include <Generated/ServerPacketHandler.h>
#include <Protocol/ProtocolVersion.h>
#include "Network/Session/PlayerSession.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Network/Session/SessionIdMaker.h"
#include "Service/Player/PlayerManager/PlayerManager.h"
#include "Shard/ShardManager.h"
#include "Network/ServerConfig.h"
#include "Service/Room/Room.h"

/*---------------------------------
   PlayerSessionLifecycleService
---------------------------------*/

PlayerSessionLifecycleService::PlayerSessionLifecycleService(SessionManager& sessionManager,
    PlayerManager& playerManager, ShardManager& shardManager, const GameConfig& gameConfig)
        : sessionManager_(sessionManager)
        , playerManager_(playerManager)
        , shardManager_(shardManager)
        , movementUpdateHz_(gameConfig.movementUpdateHz)
        , pingIntervalMs_(gameConfig.pingIntervalMs)
{
}

void PlayerSessionLifecycleService::OnConnected(PlayerSession& session)
{
    const SessionId newSessionId = SessionIdMaker::Next();
    
    session.AssignId(newSessionId);
    sessionManager_.Add(newSessionId, session.AsShared<PlayerSession>());
    
    // TEMP: SessionId == PlayerID (DB가 붙지 않는 동안은 해당 정책 사용)
    const PlayerId newPlayerId = newSessionId;
    sessionManager_.BindPlayer(newSessionId, newPlayerId);
    
    auto player = playerManager_.Create(newPlayerId);
    if (!player) {
        session.Disconnect(L"Failed to create player");
        return;
    }
    
    player->id_ = newPlayerId;
    player->sessionId_ = newSessionId;
    player->TrySetNickname(std::string{"Player"} + std::to_string(newPlayerId));
    
    session.SetPlayerId(newPlayerId);
    session.SetState(PlayerSessionState::Handshaking);
}

void PlayerSessionLifecycleService::OnDisconnected(PlayerSession& session)
{
    session.SetState(PlayerSessionState::Closing);
    
    const SessionId sessionId = session.Id();
    PlayerId playerId = session.GetPlayerId();
    if (playerId == 0) {
        if (!sessionManager_.TryGetPlayerId(sessionId, playerId)) {
            consoleLogger->Log(Color::Yellow, L"[PlayerSessionLifecycleService] Failed to get playerId for sessionId %llu during disconnection\n", sessionId);
            return;
        }
    }
    
    if (playerId != 0) {
        
        auto playerRef = playerManager_.Find(playerId);
        if (playerRef) {
            auto* playerManager = &playerManager_;
            auto* shardManager = &shardManager_;
            auto* sessionManager = &sessionManager_;
            const RoomId roomId = playerRef->roomId_;
            const ShardId shardId = playerRef->shardId_;
            
            if (roomId != 0 and shardId != 0) {
                shardManager->Enqueue(shardId, [playerManager, shardManager, sessionManager, shardId, roomId, playerId, sessionId]()
                {
                    auto* shard = shardManager->GetShard(shardId);
                    if (!shard) return;
                    
                    auto room = shard->FindRoom(roomId);
                    if (!room) return;
                    
                    room->Leave(playerId);
                    
                    playerManager->Remove(playerId);
                    sessionManager->UnbindPlayer(playerId);
                    sessionManager->RemoveBySessionId(sessionId);
                });
            }
        }
    }
}

bool PlayerSessionLifecycleService::HandleHandshake(PlayerSession& session, const se::auth::C_HandshakeReq& pkt)
{
    if (session.GetState() != PlayerSessionState::Handshaking) {
        return false;   // 현재 세션 상태가 Handshaking이 아닌 경우, 핸드쉐이크 요청을 처리할 수 없음
    }
    
    if (pkt.client_protocol_version() != se::protocol::kProtocolVersion) {
        SendHandshakeRes(session, false, se::common::ERR_INVALID_PROTOCOL_VERSION, "Protocol version mismatch");
        
        session.Disconnect(L"Incompatible protocol version");
        return false;
    }
    
    SendHandshakeRes(session, true, se::common::ERR_NONE, "OK");
    
    session.SetState(PlayerSessionState::InLobby);
    return true;
}

void PlayerSessionLifecycleService::SendHandshakeRes(PlayerSession& session, bool success, se::common::ErrorCode code, const std::string& message)
{
    se::auth::S_HandshakeRes handshakeRes;
    handshakeRes.set_success(success);
    
    auto* result = handshakeRes.mutable_result();
    result->set_code(code);
    result->set_message(message);
    
    if (session.GetPlayerId() != 0) 
        handshakeRes.set_session_player_id(session.GetPlayerId());
    
    auto* config = handshakeRes.mutable_config();
    config->set_movement_update_hz(movementUpdateHz_);
    config->set_ping_interval_ms(pingIntervalMs_);
    
    auto buffer = ServerPacketHandler::MakeSendBuffer(handshakeRes);
    if (buffer) {
        session.Send(buffer);
    }
}
