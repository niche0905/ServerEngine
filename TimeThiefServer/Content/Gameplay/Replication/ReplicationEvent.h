#pragma once
#include <variant>
#include "Content/Object/ObjectId.h"
#include "ReplicationTypes.h"
#include <vector>
#include <common/common_types.pb.h>
#include "Content/Object/ObjectEnum.h"
#include "Physics/Narrowphase/IntersectUtil.h"

enum class RepEventType : uint8
{
    None = 0,
    
    Spawn,
    Despawn,
    
    HealthChange,
    MaxHealthChange,
    MoneyChange,
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
    PlayerId playerId{0};   // optional (Owner Client 에게만 발생하는 이벤트의 경우, 예: HealthChangeEvent)
    ObjectId source{};
    ObjectId target{};      // optional
};

struct SpawnEvent
{
    ObjectType type{ObjectType::OBJ_NONE};
    uint32 templateId{0};
    // 동적으로 생성하는 것들은 아이템, 투사체로 한정 될 것이므로 일단 보류
    SE::Math::Vector3 position{};
    SE::Math::Vector3 velocity{};
    uint32 amount;
};

struct HealthChangeEvent
{
    int32 newHealth{0};
    int32 deltaHealth{0};
};

struct MaxHealthChangeEvent
{
    int32 newMaxHealth{0};
    int32 newHealth{0};
};

struct MoneyChangeEvent
{
    int32 newMoney{0};
    int32 deltaMoney{0};
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
    SpawnEvent,
    HealthChangeEvent,
    MaxHealthChangeEvent,
    MoneyChangeEvent,
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
