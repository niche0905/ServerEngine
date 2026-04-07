#pragma once
#include "Service/Player/Player.h"
#include "Network/Session/PlayerSession.h"
#include "Network/Buffer/SendBuffer.h"

class PlayerSession;
class SendBuffer;

/*------------------
   SessionManager
------------------*/
//
// SessionManager는 모든 네트워크 세션을 관리합니다.
// 컨텐츠(게임 서버) 코드에서 사용하기 위한 세션 관리 기능을 제공합니다.
//

class SessionManager
{
private:
   using SessionRef = std::shared_ptr<PlayerSession>;
   using SendBufferRef = std::shared_ptr<SendBuffer>;
   
public:
   void Add(SessionId sessionId, const SessionRef& session);   // 연결 생성
   // void Remove(SessionRef session);
   void RemoveBySessionId(SessionId sessionId);                // 연결 종료
   
   void Clear();
   
   SessionRef FindBySessionId(SessionId sessionId) const;
   SessionRef FindByPlayerId(PlayerId playerId) const;
   
   // 로그인/인증 이후 바인딩
   bool BindPlayer(SessionId sessionId, PlayerId playerId);
   void UnbindPlayer(PlayerId playerId);
   
   // 바인딩 조회
   bool TryGetSessionId(PlayerId plaeyrId, SessionId& outSessionId) const;
   bool TryGetPlayerId(SessionId sessionId, PlayerId& outPlayerId) const;
   
   // 유틸리티
   size_t GetSessionCount() const;
   size_t GetBoundPlayerCount() const;
   
   // 송신
   void Broadcast(SendBufferRef sendBuffer);
   
   // 스냅샷 (lock을 오래 잡지 않으려고)
   std::vector<SessionRef> SnapshotSessions() const;
   
private:
   mutable std::mutex mutex_;   // 세션 리스트 보호용 뮤텍스
   
   std::unordered_map<SessionId, SessionRef> sessionsById_;       // 세션 ID -> 세션 참조
   std::unordered_map<PlayerId, SessionId> sessionIdByPlayerId_;  // 플레이어 ID -> 세션 ID (바인딩 정보)
   std::unordered_map<SessionId, PlayerId> playerIdBySessionId_;  // 세션 ID -> 플레이어 ID (바인딩 정보)
    
};
