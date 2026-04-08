#pragma once
#include "TypesDef.h"

struct PlayerRoute
{
    PlayerId playerId = 0;
    RoomId roomId = 0;
    ShardId shardId = 0;
    
    bool IsValid() const
    {
        return playerId != 0 and roomId != 0 and shardId != 0;
    }
};