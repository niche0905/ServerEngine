#pragma once
#include "Content/Object/ObjectId.h"

enum class DespawnReason : uint8
{
    Unknown = 0,
    
    LifetimeExpired,
    Script,
    RoomReset,
};

struct SpawnMeta
{
    uint64 expireAtMs{0};       // 0이면 만료 없음 (등록 안하는 게 보통)
};
