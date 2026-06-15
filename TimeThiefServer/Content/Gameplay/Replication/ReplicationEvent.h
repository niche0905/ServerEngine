#pragma once
#include <variant>
#include "Content/Object/ObjectId.h"
#include "ReplicationTypes.h"
#include <vector>
#include <common/common_types.pb.h>
#include "Content/Object/ObjectEnum.h"
#include "Data/Tables/UpgradeTable.h"
#include "Physics/Narrowphase/IntersectUtil.h"

enum class RepEventType : uint8
{
    None = 0,
    
    Spawn,
    Despawn,
    
    Death,
    Respawn,
    
    HealthChange,
    MaxHealthChange,
    MoneyChange,
    
    Explosion,              // 폭발 이벤트 (폭발이 발생한 위치, 범위 등 포함)
    
    PickupItem,
    ItemChange,
    StoreEntryBlock,
    EquipItem,
    UseItem,
    ChestInteract,
    
    Jump,
    DoubleJump,
    JumpLand,
    Crouch,
    WireLaunch,
    WireAction,
    WireActionEnd,

    UseSkill,               // 스킬 사용 이벤트 (VFX/Sound 등 클라이언트 연출 트리거)
    
    Aim,                    // 조준 상태 변경 이벤트 (조준 시작, 조준 해제 등 포함) 
    Fire,                   // 발사 이벤트 (무기 종류, 발사 위치, 방향 등 포함)
    Reload,                 // 재장전 이벤트 (무기 종류, 재장전 시작할 때 모션)
    WeaponChange,           // 무기 변경 이벤트 (새로운 무기 ID 등 포함)
    Hit,                    // 히트 이벤트 (공격이 명중한 경우, 명중 위치, 피해량 등 포함)
    
    GrenadeThrow,
    GrenadeMoveSync,
    GrenadeExplosion,
    
    Attack,
    MonsterFire,
    MonsterImpact,
    
    WeaponStatChange,       // 무기 스탯 변경 이벤트 (무기 ID, 변경된 스탯과 그 값 등 포함)
    
    KillPlayer,             // 플레이어 킬 이벤트 (킬한 플레이어 ID, 킬당한 플레이어 ID)
    
    ZoneChange,             // 존 변화 이벤트 (존이 시간에 따라 좁혀지는 것)
    ZoneFlow,               // 테스트 용 (Zone이 Stop 되거나 Start 될 때)
    
    
    // ItemUseStart,
    // ItemUseCancel,
    // ItemUseComplete,
};

struct RepEventHeader
{
    RepEventType type{RepEventType::None};
    uint64 timeMs{0};
    TickSeq tick{0};
    PlayerId playerId{0};           // optional (Owner Client 에게만 발생하는 이벤트의 경우, 예: HealthChangeEvent)
    PlayerId exceptPlayerId{0};     // optional (모든 클라이언트에게 발생하는 이벤트 중, 특정 플레이어에게만 발생하지 않는 이벤트의 경우)
    ObjectId source{};
    ObjectId target{};              // optional
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

struct RespawnEvent
{
    SE::Math::Vector3 position{};
    float yaw{0};
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

struct EquipItemEvent
{
    uint32 itemId{0};
};

struct StoreEntryBlockEvent
{
    uint32 entryId{0};
    bool blocked{false};
};

struct UseItemEvent
{
    uint32 itemId{0};
};

struct CrouchEvent
{
    bool isCrouching{false};
};

struct WireLaunchEvent
{
    SE::Math::Vector3 startPos{};
    SE::Math::Vector3 direction{};
};

struct WireActionEvent
{
    SE::Math::Vector3 anchorPoint{};
};

enum class UseSkillDetailType : uint8
{
    None = 0,
    TimeAccel,
    TimeAfterImage,
    TimeRewind,
};

struct UseSkillEvent
{
    uint32 skillId{0};
    uint32 slotIndex{0};
    uint32 durationMs{0};
    uint64 startedAtMs{0};

    UseSkillDetailType detailType{UseSkillDetailType::None};

    uint32 fireRateBonusPercent{0};
    uint32 moveSpeedBonusPercent{0};

    SE::Math::Vector3 startPos{};
    SE::Math::Vector3 direction{};
    float moveSpeed{0.0f};

    uint32 rewindDurationMs{0};
    uint32 invulnerableDurationMs{0};
    int32 targetHealth{0};
    SE::Math::Vector3 targetPosition{};
};

struct AimEvent
{
    bool isAimed{false};
};

struct FireEvent
{
    uint32 weaponId{0};
    uint32 shotSeed{0};
    SE::Math::Vector3 startPos{};       // muzzle
    SE::Math::Vector3 direction{};      // normalized
};

struct ReloadEvent
{
    uint32 weaponId{0};
};

struct WeaponChangedEvent
{
    uint32 newWeaponId{0};
};

struct HitEvent
{
    SE::Math::Vector3 hitPos{};
    int32 damage{0};
};

struct GrenadeThrowEvent
{
    uint32 grenadeType{0};
    SE::Math::Vector3 startPos{};
    SE::Math::Vector3 direction{};
};

struct GrenadeMoveSyncEvent
{
    SE::Math::Vector3 position{};
    SE::Math::Vector3 rotation{};
    SE::Math::Vector3 velocity{};
};

struct GrenadeExplosionEvent
{
    SE::Math::Vector3 explosionPos{};
};

struct AttackEvent
{
    uint32 attackId;
};

struct MonsterFireEvent
{
    uint32 attackId{0};
    SE::Math::Vector3 origin{};
    SE::Math::Vector3 direction{};
    float range{0};
};

struct MonsterImpactEvent
{
    uint32 attackId{0};
    SE::Math::Vector3 position{};
};

struct AimChangedEvent
{
    bool isAimed{false};
};

struct WeaponStatChangeEvent
{
    uint32 weaponId{0};
    WeaponStatModifier modifier{};
};

struct ZoneChangeEvent
{
    SE::Math::Vector3 center{};
    float radius{0};
    float waitDuration{0};
    float shrinkDuration{0};
};

struct ZoneFlowEvent
{
    bool flowing{false};
};

using RepEventPayload = std::variant<
    std::monostate,
    SpawnEvent,
    RespawnEvent,
    HealthChangeEvent,
    MaxHealthChangeEvent,
    MoneyChangeEvent,
    ExplosionEvent,
    ItemChangeEvent,
    EquipItemEvent,
    StoreEntryBlockEvent,
    UseItemEvent,
    CrouchEvent,
    WireLaunchEvent,
    WireActionEvent,
    UseSkillEvent,
    AimEvent,
    FireEvent,
    ReloadEvent,
    HitEvent,
    GrenadeThrowEvent,
    GrenadeMoveSyncEvent,
    GrenadeExplosionEvent,
    AttackEvent,
    MonsterFireEvent,
    MonsterImpactEvent,
    WeaponChangedEvent,
    AimChangedEvent,
    WeaponStatChangeEvent,
    ZoneChangeEvent,
    ZoneFlowEvent
>;
// TODO: 필요한 세부 Event 타입과 페이로드 정의하기 (여 위에 추가)

struct RepEvent
{
    RepEventHeader header{};
    RepEventPayload payload;
};
