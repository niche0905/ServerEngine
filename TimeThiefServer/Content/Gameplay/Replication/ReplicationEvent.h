#pragma once
#include "Content/Object/ObjectId.h"
#include "ReplicationTypes.h"
#include <vector>

enum class RepEventType : uint8
{
    None = 0,
    
    Spawn,
    Despawn,
    
    Damage,
    Death,
    Respawn,
    
    Fire,
    ProjectileExplode,
    
    ItemUseStart,
    ItemUseCancel,
    ItemUseComplete,
    
    AttackStart,
    AttackHit,
};

struct RepEvent
{
    RepEventType type{RepEventType::None};
    ObjectId source{};
    ObjectId target{};
    
    // 추가 데이터
    int32 a{0};
    int32 b{0};
    
    TickSeq tick{0};
    uint64 timeMs{0};
};
