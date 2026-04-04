#include "pch.h"
#include "PlayerCombatComponent.h"
#include "Content/Object/Actor/Actor.h"
#include "IDamageable.h"
#include "Content/Object/Actor/Pawn.h"
#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"
#include "Service/Room/Room.h"

namespace 
{
    uint8 WeaponSlotFromWeaponId(uint32 weaponId)
    {
        switch (weaponId)
        {
        case 1:
            return 0;
        case 2:
            return 1;
        case 3:
            return 2;
        default:
            return PlayerCombatState::MaxWeaponSlots;  // 유효하지 않은 슬롯 인덱스 반환
        }
    }
    
}

/*-------------------------
   PlayerCombatComponent
-------------------------*/

void PlayerCombatComponent::Init(ObjectId owner, Pawn* ownerPawn)
{
    CombatComponent::Init(owner, ownerPawn);
    
    // TEMP: 시작 무기 3개 초기화 예
    combatState_.weapons[0] = WeaponState{ .weaponId = 1, .ammoInMag = 30, .reserveAmmo = 90, .isReloading = false };
    combatState_.weapons[1] = WeaponState{ .weaponId = 2, .ammoInMag = 8, .reserveAmmo = 25, .isReloading = false };
    combatState_.weapons[2] = WeaponState{ .weaponId = 3, .ammoInMag = 1, .reserveAmmo = 10, .isReloading = false };
    combatState_.currentWeaponSlot = 0;
}

bool PlayerCombatComponent::CanAttack(const AttackRequest& request) const
{
    if (!CombatComponent::CanAttack(request)) {
        return false;   // 기본 조건을 만족하지 않으면 공격할 수 없음
    }

    switch (request.type)
    {
    case AttackType::Melee:
        // TODO: 근접 공격에 대한 추가 조건이 있다면 여기에 작성 (예: 쿨타임)
        break;
        
    case AttackType::Hitscan:
        {
            if (request.weaponId == 0)
                return false;   // 무기 ID가 0인 경우 유효하지 않음
            
            uint8 weaponSlot = WeaponSlotFromWeaponId(request.weaponId);
            if (!IsValidWeaponSlot(weaponSlot))
                return false;
    
            const WeaponState* weaponState = GetWeaponStateBySlot(weaponSlot);
            if (!weaponState || weaponState->weaponId == 0)
                return false;   // 해당 슬롯에 무기가 없거나 유효하지 않은 무기
    
            if (weaponState->isReloading)
                return false;   // 재장전 중인 무기는 공격할 수 없음
            
            if (weaponState->ammoInMag <= 0)
                return false;   // 탄약이 없는 무기는 공격할 수 없음
        }
        break;
        
    case AttackType::Projectile:
        // TODO: 발사체 공격에 대한 추가 조건이 있다면 여기에 작성
        //       발사체(투사체)는 폭파 상황이 아닌 발사(던지는) 상황에서의 공격이고, 투사체 공격은 발사 시점에 탄약이 소비되고,
        //       폭발 시점에는 해당 Object가 직접 Damage 처리를 하는 형태로 구현할 수 있을 것...
        break;
    }
    
    return true;
}

bool PlayerCombatComponent::TryReload()
{
    WeaponState* currentWeapon = GetCurrentWeaponState();
    if (!currentWeapon)
        return false;
    
    if (currentWeapon->isReloading)
        return false;   // 이미 재장전 중인 경우
    
    constexpr int kTempMagazineCapacity = 30;   // TEMP: 모든 총의 탄창 용량을 30으로 가정
    
    if (currentWeapon->ammoInMag >= kTempMagazineCapacity)
        return false;   // 탄창이 이미 가득 찬 경우
    
    if (currentWeapon->reserveAmmo <= 0)
        return false;   // 예비 탄약이 없는 경우
    
    const int needAmmo = kTempMagazineCapacity - currentWeapon->ammoInMag;
    const int reloadAmmo = std::min(needAmmo, currentWeapon->reserveAmmo);
    
    currentWeapon->ammoInMag += reloadAmmo;
    currentWeapon->reserveAmmo -= reloadAmmo;
    currentWeapon->isReloading = false;   // TODO: 실제 재장전 시간과 애니메이션이 필요하다면, 재장전 시작 시점에 true로 설정하고, 일정 시간 후에 false로 변경하는 로직 추가
    
    return true;
}

uint32 PlayerCombatComponent::GetHandWeaponId() const
{
    const WeaponState* currentWeapon = GetCurrentWeaponState();
    if (!currentWeapon)
        return 0;   // 현재 무기가 없는 경우 유효하지 않은 무기 ID 반환
    
    return currentWeapon->weaponId;
}

bool PlayerCombatComponent::SwitchWeapon(uint32 newWeaponId)
{
    const uint8 newSlotIndex = WeaponSlotFromWeaponId(newWeaponId);
    if (!IsValidWeaponSlot(newSlotIndex))
        return false;   // 유효하지 않은 슬롯 인덱스
    
    const WeaponState* weaponState = GetWeaponState(newSlotIndex);
    if (!weaponState || weaponState->weaponId == 0)
        return false;
    
    combatState_.currentWeaponSlot = newSlotIndex;
    return true;
}

const WeaponState* PlayerCombatComponent::GetCurrentWeaponState() const
{
    return GetWeaponState(combatState_.currentWeaponSlot);
}

WeaponState* PlayerCombatComponent::GetCurrentWeaponState()
{
    return GetWeaponState(combatState_.currentWeaponSlot);
}

const WeaponState* PlayerCombatComponent::GetWeaponState(uint32 weaponId) const
{
    const uint8 slotIndex = WeaponSlotFromWeaponId(weaponId);
    if (!IsValidWeaponSlot(slotIndex)) 
        return nullptr;
    
    return &combatState_.weapons[slotIndex];
}

WeaponState* PlayerCombatComponent::GetWeaponState(uint32 weaponId)
{
    const uint8 slotIndex = WeaponSlotFromWeaponId(weaponId);
    if (!IsValidWeaponSlot(slotIndex))
        return nullptr;
    
    return &combatState_.weapons[slotIndex];
}

const WeaponState* PlayerCombatComponent::GetWeaponStateBySlot(uint8 slotIndex) const
{
    if (!IsValidWeaponSlot(slotIndex))
        return nullptr;    // 유효하지 않은 슬롯 인덱스
    
    return &combatState_.weapons[slotIndex];
}

WeaponState* PlayerCombatComponent::GetWeaponStateBySlot(uint8 slotIndex)
{
    if (!IsValidWeaponSlot(slotIndex))
        return nullptr;    // 유효하지 않은 슬롯 인덱스
    
    return &combatState_.weapons[slotIndex];
}

uint8 PlayerCombatComponent::GetCurrentWeaponSlot() const
{
    if (!IsValidWeaponSlot(combatState_.currentWeaponSlot))
        return PlayerCombatState::MaxWeaponSlots;  // 유효하지 않은 슬롯 인덱스 반환
    
    return combatState_.currentWeaponSlot;
}

uint32 PlayerCombatComponent::GetCurrentWeaponId() const
{
    if (!IsValidWeaponSlot(combatState_.currentWeaponSlot))
        return 0;   // 유효하지 않은 무기 ID 반환
    
    return combatState_.weapons[combatState_.currentWeaponSlot].weaponId;
}

bool PlayerCombatComponent::ExecuteAttack(AttackRequest& request)
{
    bool hit = false;
    switch (request.type)
    {
    case AttackType::Melee:
        break;
    case AttackType::Hitscan:
        {
            switch (request.weaponId)
            {
            // TODO: 무기 ID Enum 값으로 변경...
            case 1:
                {
                    // TODO: 여기서 무기에 따른 데이터 처리하기 (예: 데미지, 사거리, 탄착 효과 등등)
                    request.damage = 12;
                    request.range = 1000.0f;
                    hit = FireHitscan(request);
                }
                break;
            }
        }
        break;
    case AttackType::Projectile:
        break;
    default:
        break;
    }
    
    return hit;
}

bool PlayerCombatComponent::FireHitscan(const AttackRequest& request)
{
    uint8 weaponSlot = WeaponSlotFromWeaponId(request.weaponId);
    if (!ConsumeAmmo(weaponSlot, 1))
        return false;
    
    Pawn* ownerPawn = GetOwnerPawn();
    if (!ownerPawn)
        return false;
    
    auto room = ownerPawn->GetRoom();
    if (!room)
        return false;
    
    SE::Math::Vector3 dir = request.direction.Normalized();
    SE::Physics::Ray ray(request.origin, dir, request.range);
    
    Actor* victim = nullptr;
    SE::Physics::Hit::HitResult hitInfo{};
    
    if (room->TraceHit(ray, hitInfo)) {
        if (hitInfo.hit) {
            victim = hitInfo.actor;
        }
    }
    
    if (victim == nullptr) 
        return false;   // 히트한 Actor가 없는 경우
    
    IDamageable* damageable = dynamic_cast<IDamageable*>(victim);
    if (!damageable)
        return false;
    
    DamageContext ctx;
    ctx.attacker = ownerPawn->GetId();
    ctx.type = DamageType::Ranged;
    ctx.source = DamageSource::Weapon;
    
    DamageResult damageResult = damageable->ApplyDamage(room->GetObjectManager(), request.damage, ctx);
    room->HandleDamageResult(GetOwnerPawn(), victim, hitInfo, ctx, damageResult);
    
    return true;
}

bool PlayerCombatComponent::FireMelee(const AttackRequest& request)
{
    
    return false;
}

bool PlayerCombatComponent::IsValidWeaponSlot(uint8 slotIndex) const
{
    return slotIndex < combatState_.weapons.size();
}

bool PlayerCombatComponent::ConsumeAmmo(uint8 slotIndex, int amount)
{
    WeaponState* weaponState = GetWeaponStateBySlot(slotIndex);
    if (!weaponState)
        return false;
    
    if (weaponState->ammoInMag < amount)
        return false;
    
    weaponState->ammoInMag -= amount;
    return true;
}

bool PlayerCombatComponent::CanFireWeapon(uint8 slotIndex) const
{
    const WeaponState* weaponState = GetWeaponStateBySlot(slotIndex);
    if (!weaponState)
        return false;
    
    if (weaponState->weaponId == 0)
        return false;
    
    if (weaponState->isReloading)
        return false;
    
    return weaponState->ammoInMag > 0;
}
