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
    
    Explosion,              // 폭발 이벤트 (폭발이 발생한 위치, 범위 등 포함)
    
    ItemChange,
    
    Fire,                   // 발사 이벤트 (무기 종류, 발사 위치, 방향 등 포함)
    
    ZoneFlow,               // 테스트 용 (Zone이 Stop 되거나 Start 될 때)
    
    
    // Death,
    // Respawn,
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
    SE::Math::Vector3 position{};
    SE::Math::Vector3 velocity{};       // optional, 투사체 등 움직이는 오브젝트에 대해서만 사용
    float yaw{0};                       // optional, 회전이 필요한 오브젝트에 대해서만 사용 (예: Player)
    uint32 amount;                      // optional, 아이템 스택 수량 등 필요에 따라 사용
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

struct ExplosionEvent
{
    SE::Math::Vector3 explosionPos{};       // 폭발이 발생한 위치(중점)
    float explosionRadius{0};               // 폭발 범위(반지름)
};

struct ItemChangeEvent
{
    uint32 itemId{0};
    int32 newCount{0};
    int32 deltaCount{0};
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



struct ZoneFlowEvent
{
    bool flowing{false};
};

using RepEventPayload = std::variant<
    std::monostate,
    SpawnEvent,
    HealthChangeEvent,
    MaxHealthChangeEvent,
    MoneyChangeEvent,
    ExplosionEvent,
    ItemChangeEvent,
    FireEvent,
    WeaponChangedEvent,
    AimChangedEvent,
    ZoneFlowEvent
>;
// TODO: 필요한 세부 Event 타입과 페이로드 정의하기 (여 위에 추가)

struct RepEvent
{
    RepEventHeader header{};
    RepEventPayload payload;
};
