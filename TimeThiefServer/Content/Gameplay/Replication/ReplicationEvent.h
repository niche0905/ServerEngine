#pragma once
#include <variant>
#include "Content/Object/ObjectId.h"
#include "ReplicationTypes.h"
#include <vector>
#include <common/common_types.pb.h>

#include "Physics/Narrowphase/IntersectUtil.h"

enum class RepEventType : uint8
{
    None = 0,
    
    Spawn,
    Despawn,
    
    // Damage,
    // Death,
    // Respawn,
    //
    // Fire,
    // ProjectileExplode,
    //
    // ItemUseStart,
    // ItemUseCancel,
    // ItemUseComplete,
    //
    // AttackStart,
    // AttackHit,
};

struct RepEventHeader
{
    RepEventType type{RepEventType::None};
    uint64 timeMs{0};
    TickSeq tick{0};
    ObjectId source{};
    ObjectId target{};      // optional
};

struct FireEvent
{
    uint32 weaponId{0};
    uint32 shotSeed{0};
    SE::Math::Vector3 startPos{};       // muzzle
    SE::Math::Vector3 direction{};      // normalized
};

struct WeaponChangedEvent
{
    uint32 newWeaponId{0};
};

struct AimChangedEvent
{
    bool isAimed{false};
};

using RepEventPayload = std::variant<
    std::monostate,
    FireEvent,
    WeaponChangedEvent,
    AimChangedEvent
>;
// TODO: 필요한 세부 Event 타입과 페이로드 정의하기 (여 위에 추가)

struct RepEvent
{
    RepEventHeader header{};
    RepEventPayload payload;
};
