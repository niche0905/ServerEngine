#pragma once
#include "Service/Player/Player.h"
#include "Service/MatchMaking/MatchMakingQueue.h"

class ShardManager;
class PlayerManager;
class SessionManager;
class PlayerSession;

/*--------------
   MatchMaker
--------------*/
//
// MatchMaker는 매칭 큐에 플레이어를 추가/제거하고, 매칭이 가능한 플레이어가 있는지 확인하여 매칭을 시도하는 역할을 담당한다.
//

class MatchMaker
{
public:
    MatchMaker() = default;
    ~MatchMaker() = default;
    
    MatchMaker(const MatchMaker&) = delete;
    MatchMaker& operator=(const MatchMaker&) = delete;
    
public:
    bool Init(SessionManager& sessionManager, PlayerManager& playerManager, ShardManager& shardManager);
    
public:
    bool Enqueue(PlayerId playerId);
    bool Cancel(PlayerId playerId);
    
    void TryMatch();
    
private:
    SessionManager*         sessionManager_ = nullptr;       // non-owning
    PlayerManager*          playerManager_ = nullptr;        // non-owning
    ShardManager*           shardManager_ = nullptr;         // non-owning
    
private:
    static constexpr size_t kMatchSize = 2; // TODO: 이 값도 밖에서 받아오는 config 값 사용하기
    
    MatchMakingQueue queue_;
    
};

// TODO: TTSA 완성하고 제대로 의존성 제거 후 아래 전역 변수 정의 지우기
extern MatchMaker g_MatchMaker;
