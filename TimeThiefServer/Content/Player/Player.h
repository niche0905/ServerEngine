#pragma once
#include "PlayerState.h"

using PlayerId = uint64;
using SessionId = uint64;
using PawnId = uint32;

/*----------
   Player
----------*/
//
// Player는 게임 플레이어를 나타냅니다.
// 얇게 구현된 객체로, 플레이어의 고유 ID, 세션 ID, 조종하는 Pawn의 ID, 그리고 플레이어 상태를 포함합니다.
//

class Player
{
public:
    PlayerId id_ = 0;               // 플레이어 고유 ID (DB ID)
    SessionId sessionId_ = 0;       // 플레이어와 연결된 세션 ID
    PawnId pawnId_ = 0;             // 플레이어가 조종하는 Pawn의 ID
    
    PlayerState state_;
    
};
