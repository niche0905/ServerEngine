#pragma once

using SessionId = uint64;

class SessionIdMaker
{
public:
    static SessionId Next()
    {
        return nextSessionId.fetch_add(1, std::memory_order_relaxed);
    }

private:
    // 0은 유효하지 않은 세션 ID로 간주하므로 1부터 시작
    static inline std::atomic<SessionId> nextSessionId{1};
    
};
