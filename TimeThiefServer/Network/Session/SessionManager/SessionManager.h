#pragma once
#include "Network/Session/PlayerSession.h"
#include "Network/Buffer/SendBuffer.h"

/*------------------
   SessionManager
------------------*/
//
// SessionManager는 모든 네트워크 세션을 관리합니다.
// 컨텐츠(게임 서버) 코드에서 사용하기 위한 세션 관리 기능을 제공합니다.
//

class PlayerSession;
class SendBuffer;

class SessionManager
{
private:
   using SessionRef = std::shared_ptr<PlayerSession>;
   using SendBufferRef = std::shared_ptr<SendBuffer>;
   
public:
   void Add(SessionRef session);
   void Remove(SessionRef session);
   
   void Clear();
   
   void Broadcast(SendBufferRef sendBuffer);
   size_t GetSessionCount() const;
   
private:
   std::mutex mutex_;   // 세션 리스트 보호용 뮤텍스
   std::unordered_set<SessionRef> sessions_; // 활성 세션 리스트
    
};

extern SessionManager g_SessionManager;
