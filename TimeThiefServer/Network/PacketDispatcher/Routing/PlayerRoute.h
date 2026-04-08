#pragma once
#include "TypesDef.h"

struct PlayerRoute
{
    PlayerId playerId = 0;
    RoomId roomId = 0;
    ShardId shardId = 0;
    
    bool IsValid() const
    {
        // TODO: shardId는 0이 될 수 있는 경우가 존재할 수 있으므로, shardId에 대한 유효성 검사는 shard 정책이 확정된 이후에 다시 고민하기 (현재는 shardId가 0인 경우는 유효하지 않은 것으로 간주하는 구조로)
        return playerId != 0 and roomId != 0 and shardId != 0;
    }
};