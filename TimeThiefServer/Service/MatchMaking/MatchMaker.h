#pragma once
#include "Service/Player/Player.h"
#include "Service/MatchMaking/MatchMakingQueue.h"

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
    bool Enqueue(PlayerId playerId);
    bool Cancel(PlayerId playerId);
    
    void TryMatch();
    
private:
    static constexpr size_t kMatchSize = 2;
    
    MatchMakingQueue queue_;
    
};

// TODO: TTSA 완성하고 제대로 의존성 제거 후 아래 전역 변수 정의 지우기
extern MatchMaker g_MatchMaker;
