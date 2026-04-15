#pragma once

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

struct AttackRequest
{
    AttackType type = AttackType::None;
    Actor* instigator = nullptr;   // 공격을 가한 Actor (공격자)
   
    SE::Math::Vector3 origin{};
    SE::Math::Vector3 direction{};
   
    float range = 0.0f;
    int32 damage = 0;   // 공격이 가할 피해량 (향후 무기 시스템이 구현되면 활용)
    
    uint32 weaponId = 0;   // 공격에 사용된 무기의 ID (향후 무기 시스템이 구현되면 활용)
};

struct WeaponState
{
    uint32 weaponId = 0;
    int ammoInMag = 0;
    int reserveAmmo = 0;    // TODO: 제거핮자 (탄은 무한)
    bool isReloading = false;
};

// TODO: WeaponConfig 추가하기 (아마 GameShared로 빼야할 듯)
//       어떤 총기 종류가 있고, 해당 종류는 어떤 Type의 공격이고
//       탄창은 얼마나 들어가고, 재장전 시간은 얼마나 걸리고,
//       사거리는 어느 정도이고 damage는 어느 정도인지

struct PlayerCombatState
{
    static constexpr size_t MaxWeaponSlots = 3;
    
    std::array<WeaponState, MaxWeaponSlots> weapons{};
    uint8 currentWeaponSlot = 0;
};
