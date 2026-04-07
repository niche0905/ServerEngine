#include "pch.h"
#include "PlayerSessionLifecycleService.h"

#include <Generated/ServerPacketHandler.h>
#include <Protocol/ProtocolVersion.h>

#include "Network/Session/PlayerSession.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Network/Session/SessionIdMaker.h"
#include "Service/Player/PlayerManager/PlayerManager.h"
#include "Shard/ShardManager.h"

/*---------------------------------
   PlayerSessionLifecycleService
---------------------------------*/

PlayerSessionLifecycleService::PlayerSessionLifecycleService(SessionManager& sessionManager,
    PlayerManager& playerManager, ShardManager& shardManager)
        : sessionManager_(sessionManager)
        , playerManager_(playerManager)
        , shardManager_(shardManager)
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
            const RoomId roomId = playerRef->roomId_;
            const ShardId shardId = playerRef->shardId_;
            
            if (roomId != 0 and shardId != 0) {
                // TODO: ShardManager에 Room 퇴장(Leave) 요청하기
                //       shardManager_.Enqueue(shardId, [roomId, playerId]()...
                
                // TODO: 다음 Player Manager에서 제거하는 것은 굳이 해야하나? 안해도 되지 않을까...? 참조할 놈도 없을 텐데
            }
        }
    }
    
    sessionManager_.UnbindPlayer(sessionId);
    sessionManager_.RemoveBySessionId(sessionId);
}

bool PlayerSessionLifecycleService::HandleHandshake(PlayerSession& session, const se::auth::C_HandshakeReq& pkt)
{
    if (session.GetState() != PlayerSessionState::Handshaking) {
        return false;   // 현재 세션 상태가 Handshaking이 아닌 경우, 핸드쉐이크 요청을 처리할 수 없음
    }
    
    if (pkt.client_protocol_version() != se::protocol::kProtocolVersion) {
        se::auth::S_HandshakeRes handshakeRes;
        handshakeRes.set_success(false);
        
        auto* result = handshakeRes.mutable_result();
        result->set_code(se::common::ERR_INVALID_PROTOCOL_VERSION);
        result->set_message("Protocol version mismatch");
        
        if (session.GetPlayerId() != 0) 
            handshakeRes.set_session_player_id(session.GetPlayerId());
        
        auto* config = handshakeRes.mutable_config();
        config->set_movement_update_hz(10);   // TODO: config에서 읽어오기
        config->set_ping_interval_ms(1000);    // TODO: config에서 읽어오기
        
        auto buffer = ServerPacketHandler::MakeSendBuffer(handshakeRes);
        if (buffer) {
            session.Send(buffer);
        }
        
        session.Disconnect(L"Incompatible protocol version");
        return false;
    }
    
    {
        se::auth::S_HandshakeRes handshakeRes;
        handshakeRes.set_success(true);
        
        auto* result = handshakeRes.mutable_result();
        result->set_code(se::common::ERR_NONE);
        result->set_message("OK");
        
        if (session.GetPlayerId() != 0) 
            handshakeRes.set_session_player_id(session.GetPlayerId());
        
        auto* config = handshakeRes.mutable_config();
        config->set_movement_update_hz(10);   // TODO: config에서 읽어오기
        config->set_ping_interval_ms(1000);    // TODO: config에서 읽어오기
        
        auto buffer = ServerPacketHandler::MakeSendBuffer(handshakeRes);
        if (buffer) {
            session.Send(buffer);
        }
    }
    
    session.SetState(PlayerSessionState::InLobby);
    return true;
}
