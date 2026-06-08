#pragma once
#include "Content/Object/ObjectId.h"
#include "Data/Tables/WeaponTable.h"
#include "Physics/Collider/Collider.h"
#include "Utils/Types.h"

class Actor;

struct AttackContext
{
    Actor* instigator = nullptr;   // 공격을 가한 Actor (공격자)
};

enum class AttackType
{
    None,
    Melee,
    Hitscan,
    Projectile,
 };

enum class CombatEventType
{
    None,
    
    CatMelee,   // 근접 공격
    CatClaw,    // 근접 공격 1
    CatBite,    // 근접 공격 2
    CatRange,   // 원거리 공격
    CatCannon,  // 원거리 공격 1
    CatCannonCastStart,
    CatCannonFire,
    CatCannonCancel,

    MinionMelee,
    MinionLeftAttack,
    MinionRightAttack,

    BossGroundSlam,
    BossBurstCharge,
    BossBurstChargeStart,
    BossBurstExplode,
    
};

struct AttackRequest
{
    AttackType          type = AttackType::None;
    Actor*              instigator = nullptr;   // 공격을 가한 Actor (공격자)
   
    SE::Math::Vector3   origin{};
    SE::Math::Vector3   direction{};
   
    float               range = 0.0f;
    int32               damage = 0;   // 공격이 가할 피해량 (향후 무기 시스템이 구현되면 활용)
    
    uint32              weaponId = 0;   // 공격에 사용된 무기의 ID (향후 무기 시스템이 구현되면 활용)
    uint32              shotSeed = 0;   // 총알 발사 시의 랜덤 시드 (샷건 총알 궤적 계산 등에 활용)
};

struct MeleeAttackDesc
{
    ObjectId attackerId;
    CombatEventType attackType;
    
    SE::Physics::Collider* collider = nullptr;
    int32 damage = 0;
    
    bool hitPlayersOnly = true;
};

struct WeaponState
{
    uint32              weaponId = 0;
    int                 ammoInMag = 0;
    bool                isReloading = false;
    uint64              reloadToken = 0;
    TimePoint           nextAllowedFireTime{};
};

struct WeaponSlotState
{
    WeaponState         runtime{};
    WeaponStat          stat{};
    bool                dirty = true;
};

// TODO: WeaponConfig 추가하기 (아마 GameShared로 빼야할 듯)
//       어떤 총기 종류가 있고, 해당 종류는 어떤 Type의 공격이고
//       탄창은 얼마나 들어가고, 재장전 시간은 얼마나 걸리고,
//       사거리는 어느 정도이고 damage는 어느 정도인지

struct PlayerCombatState
{
    std::array<WeaponSlotState, MaxWeaponSlots> slots{};
    uint8 currentWeaponSlot = 0;
};
