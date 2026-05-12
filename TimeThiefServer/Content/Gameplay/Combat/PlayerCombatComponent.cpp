#include "pch.h"
#include "PlayerCombatComponent.h"
#include "Content/Object/Actor/Actor.h"
#include "IDamageable.h"
#include "Content/Object/Actor/Pawn.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Physics/Ray/Ray.h"
#include "Physics/Ray/RaycastHit.h"
#include "Service/Room/Room.h"
#include "Utils/Random/WeightedRandom.h"

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
            return MaxWeaponSlots;  // 유효하지 않은 슬롯 인덱스 반환
        }
    }
    
}

/*-------------------------
   PlayerCombatComponent
-------------------------*/

void PlayerCombatComponent::Init(BaseObject* owner)
{
    CombatComponent::Init(owner);
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
    
            const WeaponSlotState* weaponState = GetCurrentWeaponSlot();
            if (!weaponState || weaponState->runtime.weaponId != request.weaponId or weaponState->stat.common.fireType != WeaponFireType::HitScan)
                return false;   // 해당 슬롯에 무기가 없거나 유효하지 않은 무기
    
            if (weaponState->runtime.isReloading)
                return false;   // 재장전 중인 무기는 공격할 수 없음
            
            if (weaponState->runtime.ammoInMag <= 0)
                return false;   // 탄약이 없는 무기는 공격할 수 없음
        }
        break;
        
    case AttackType::Projectile:
        {
            if (request.weaponId != 3)
                return false;   // 무기 ID가 3이 아닌 경우 유효하지 않음
            
            const WeaponSlotState* weaponState = GetCurrentWeaponSlot();
            if (!weaponState or weaponState->runtime.weaponId != request.weaponId or weaponState->stat.common.fireType != WeaponFireType::Projectile)
                return false;   // 현재 무기가 발사체 무기가 아닌 경우
            
            if (weaponState->runtime.isReloading)
                return false;   // 재장전 중인 무기는 공격할 수 없음
            
            if (weaponState->runtime.ammoInMag <= 0)
                return false;   // 탄약이 없는 무기는 공격할 수 없음
        }
        break;
    }
    
    return true;
}

bool PlayerCombatComponent::TryReload()
{
    WeaponSlotState* currentWeapon = GetCurrentWeaponSlot();
    if (!currentWeapon)
        return false;
    
    if (currentWeapon->runtime.isReloading)
        return false;   // 이미 재장전 중인 경우
    
    const int magCapacity = currentWeapon->stat.common.magCapacity;
    if (currentWeapon->runtime.ammoInMag >= magCapacity)
        return false;   // 탄창이 이미 가득 찬 경우
    
    const int needAmmo = magCapacity - currentWeapon->runtime.ammoInMag;
    const int reloadAmmo = needAmmo;
    
    currentWeapon->runtime.ammoInMag += reloadAmmo;
    currentWeapon->runtime.isReloading = false;   // TODO: 실제 재장전 시간과 애니메이션이 필요하다면, 재장전 시작 시점에 true로 설정하고, 일정 시간 후에 false로 변경하는 로직 추가
    
    return true;
}

uint32 PlayerCombatComponent::GetHandWeaponId() const
{
    const WeaponSlotState* currentWeapon = GetCurrentWeaponSlot();
    if (!currentWeapon)
        return 0;   // 현재 무기가 없는 경우 유효하지 않은 무기 ID 반환
    
    return currentWeapon->runtime.weaponId;
}

bool PlayerCombatComponent::SwitchWeapon(uint32 newWeaponId)
{
    const uint8 newSlotIndex = WeaponSlotFromWeaponId(newWeaponId);
    if (!IsValidWeaponSlot(newSlotIndex))
        return false;   // 유효하지 않은 슬롯 인덱스
    
    const WeaponSlotState* weaponState = GetWeaponSlotByIndex(newSlotIndex);
    if (!weaponState || weaponState->runtime.weaponId == 0)
        return false;
    
    combatState_.currentWeaponSlot = newSlotIndex;
    return true;
}

const WeaponSlotState* PlayerCombatComponent::GetCurrentWeaponSlot() const
{
    return GetWeaponSlotByIndex(combatState_.currentWeaponSlot);
}

WeaponSlotState* PlayerCombatComponent::GetCurrentWeaponSlot()
{
    return GetWeaponSlotByIndex(combatState_.currentWeaponSlot);
}

const WeaponSlotState* PlayerCombatComponent::GetWeaponSlot(uint32 weaponId) const
{
    const uint8 slotIndex = WeaponSlotFromWeaponId(weaponId);
    if (!IsValidWeaponSlot(slotIndex)) 
        return nullptr;
    
    return &combatState_.slots[slotIndex];
}

WeaponSlotState* PlayerCombatComponent::GetWeaponSlot(uint32 weaponId)
{
    const uint8 slotIndex = WeaponSlotFromWeaponId(weaponId);
    if (!IsValidWeaponSlot(slotIndex))
        return nullptr;
    
    return &combatState_.slots[slotIndex];
}

const WeaponSlotState* PlayerCombatComponent::GetWeaponSlotByIndex(uint8 slotIndex) const
{
    if (!IsValidWeaponSlot(slotIndex))
        return nullptr;    // 유효하지 않은 슬롯 인덱스
    
    return &combatState_.slots[slotIndex];
}

WeaponSlotState* PlayerCombatComponent::GetWeaponSlotByIndex(uint8 slotIndex)
{
    if (!IsValidWeaponSlot(slotIndex))
        return nullptr;    // 유효하지 않은 슬롯 인덱스
    
    return &combatState_.slots[slotIndex];
}

uint8 PlayerCombatComponent::GetCurrentWeaponIndex() const
{
    if (!IsValidWeaponSlot(combatState_.currentWeaponSlot))
        return MaxWeaponSlots;  // 유효하지 않은 슬롯 인덱스 반환
    
    return combatState_.currentWeaponSlot;
}

uint32 PlayerCombatComponent::GetCurrentWeaponId() const
{
    if (!IsValidWeaponSlot(combatState_.currentWeaponSlot))
        return 0;   // 유효하지 않은 무기 ID 반환
    
    return combatState_.slots[combatState_.currentWeaponSlot].runtime.weaponId;
}

void PlayerCombatComponent::OnWeaponUpgrade()
{
    // none
}

bool PlayerCombatComponent::ExecuteAttack(AttackRequest& request)
{
    bool hit = false;
    
    const auto* currentWeapon = GetCurrentWeaponSlot();
    if (!currentWeapon)
        return false;   // 현재 무기가 없는 경우 공격할 수 없음
    
    if (request.weaponId == 0 or request.weaponId != currentWeapon->runtime.weaponId) {
        consoleLogger->Log(Color::Red, L"PlayerCombatComp: Execcute Attack failed - Invalid weaponId in request (weaponId: %u, currentWeaponId: %u)\n", request.weaponId, currentWeapon->runtime.weaponId);
        return false;   // 무기 ID가 0이거나 현재 무기와 일치하지 않는 경우 유효하지 않음
    }
    request.damage = currentWeapon->stat.common.damage;
    request.range = currentWeapon->stat.common.range;
    
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
                    hit = FireRifle(request);
                }
                break;
                
            case 2:
                {
                    hit = FireShotgun(request);
                }
                break;
                
            }
        }
        break;
    case AttackType::Projectile:
        {
            switch (request.weaponId)
            {
            // TODO: 무기 ID Enum 값으로 변경...
            case 3:
                {
                    FireLauncher(request);
                    hit = false;    // 발사체 공격의 경우 발사 시점에는 히트 판정이 없고, 투사체가 폭발할 때 히트 판정이 이루어지도록 구현할 예정이므로, 일단 여기서는 false로 설정
                }
                break;
            }
        }
        break;
    default:
        break;
    }
    
    return true;
}

bool PlayerCombatComponent::TraceHit(const AttackRequest& request, const SE::Physics::Ray& ray, SE::Physics::Hit::HitResult& outHit)
{
    Pawn* ownerPawn = GetOwnerPawn();
    if (!ownerPawn)
        return false;
    
    auto room = ownerPawn->GetRoom();
    if (!room)
        return false;
    
    Actor* victim = nullptr;
    
    if (room->GetRoomGameSystem().GetCombatSystem().TraceHit(ray, ownerPawn->GetId(), outHit)) {
        if (outHit.hit) {
            victim = outHit.actor;
        }
    }
    
    // TODO: outHit.hit 가 true, 즉 충돌이 발생 헀다면 Hit 정보를 담아서 모든 클라이언트에게 Replicate 하기
    
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
    room->HandleDamageResult(GetOwnerPawn(), victim, outHit, ctx, damageResult);
    
    return true;
}

bool PlayerCombatComponent::FireRifle(const AttackRequest& request)
{
    if (request.weaponId == 0 or request.weaponId != GetCurrentWeaponId())
        return false;   // 무기 ID가 0이거나 현재 무기와 일치하지 않는 경우 유효하지 않음
    
    uint8 weaponSlot = WeaponSlotFromWeaponId(request.weaponId);
    if (!ConsumeAmmo(weaponSlot, 1))
        return false;
    
    SE::Math::Vector3 dir = request.direction.Normalized();
    SE::Physics::Ray ray(request.origin, dir, request.range);
    
    SE::Physics::Hit::HitResult hitInfo{};
    if (!TraceHit(request, ray, hitInfo))
        return false;   // 히트 판정 실패 (예: 사격이 벽에 막히거나, 사격이 빗나간 경우)
    
    return true;
}

bool PlayerCombatComponent::FireShotgun(const AttackRequest& request)
{
    const auto* currentWeapon = GetCurrentWeaponSlot();
    if (!currentWeapon)
        return false;
    if (currentWeapon->stat.common.category != WeaponCategory::Shotgun)
        return false;   // 현재 무기가 샷건이 아닌 경우
    
    const auto* shotgun = std::get_if<ShotgunStat>(&currentWeapon->stat.extra);
    if (!shotgun)
        return false;   // 샷건 무기의 추가 정보가 없는 경우
    
    const int pelletCount = shotgun->pelletCount;
    const float spread = shotgun->coneAngleDegrees;
    
    if (request.weaponId == 0 or request.weaponId != GetCurrentWeaponId())
        return false;
    
    uint8 weaponSlot = WeaponSlotFromWeaponId(request.weaponId);
    if (!ConsumeAmmo(weaponSlot, 1))
        return false;
    
    const SE::Math::Vector3 dir = request.direction.Normalized();
    const float spreadAngle = SE::Math::Max(0.0f, spread);
    
    bool hit = false;
    PalletPattern pattern = GeneratePalletPattern(dir, pelletCount, spreadAngle, request.shotSeed);
    for (int i = 0; i < pelletCount; ++i) { 
        const SE::Math::Vector3& pelletDir = pattern.directions[i];
        SE::Physics::Ray ray(request.origin, pelletDir, request.range);
        
        SE::Physics::Hit::HitResult hitInfo{};
        if (TraceHit(request, ray, hitInfo)) {
            hit = true;    // 펠릿 중 하나라도 히트 판정에 성공하면 전체 공격이 히트한 것으로 간주
        }
    }
    
    return hit;
}

void PlayerCombatComponent::FireLauncher(const AttackRequest& request)
{
    const auto* currentWeapon = GetCurrentWeaponSlot();
    if (!currentWeapon)
        return;
    if (currentWeapon->stat.common.category != WeaponCategory::Launcher)
        return;
    
    const auto* launcher = std::get_if<LauncherStat>(&currentWeapon->stat.extra);
    if (!launcher)
        return;   // 런처 무기의 추가 정보가 없는 경우
    
    const float projectileSpeed = launcher->projectileSpeed;
    const float explosionRadius = launcher->explosionRadius;
    
    if (request.weaponId == 0 or request.weaponId != GetCurrentWeaponId())
        return;
    
    uint8 weaponSlot = WeaponSlotFromWeaponId(request.weaponId);
    if (!ConsumeAmmo(weaponSlot, 1))
        return;
    
    Pawn* ownerPawn = GetOwnerPawn();
    if (!ownerPawn)
        return;
    
    auto room = ownerPawn->GetRoom();
    if (!room)
        return;
    
    const SE::Math::Vector3 spawnPos = request.origin;
    const SE::Math::Vector3 spawnDir = request.direction.Normalized();
    
    room->GetRoomGameSystem().GetCombatSystem().LaunchRocket(spawnPos, spawnDir, ownerPawn, request.damage, projectileSpeed, 10000, explosionRadius);
}

PlayerCombatComponent::PalletPattern PlayerCombatComponent::GeneratePalletPattern(const SE::Math::Vector3& forwardDir,
                                                                                  int palletCount, float spreadDegrees, uint32 shotSeed) const
{
    PalletPattern result;
    result.directions.reserve(palletCount);
    
    Random32 rng{shotSeed};
    const SE::Math::Vector3& w = forwardDir;
    
    const SE::Math::Vector3 helper = (SE::Math::Abs(w.x) > 0.1f) ? SE::Math::Vector3(0, 1, 0) : SE::Math::Vector3(1, 0, 0);
    const SE::Math::Vector3 u = helper.Cross(w).Normalized();
    const SE::Math::Vector3 v = w.Cross(u);
    
    const float halfAngleRad = SE::Math::DegreesToRadians(spreadDegrees * 0.5f);
    const float cosMin = std::cos(halfAngleRad);
    
    for (int i = 0; i < palletCount; ++i) {
        float r1 = rng.NextFloat01();
        float r2 = rng.NextFloat01();
        
        float cosTheta = 1.0f - r1 * (1.0f - cosMin);
        float sinTheta = std::sqrt(std::max(0.0f, 1.0f - cosTheta * cosTheta));
        float phi = 2.0f * Pi * r2;
        
        SE::Math::Vector3 dir = {
            u * (std::cos(phi) * sinTheta) +
            v * (std::sin(phi) * sinTheta) +
            w * cosTheta
        };
        
        result.directions.push_back(dir.Normalized());
    }
    
    return result;
}

bool PlayerCombatComponent::IsValidWeaponSlot(uint8 slotIndex) const
{
    return slotIndex < combatState_.slots.size();
}

bool PlayerCombatComponent::ConsumeAmmo(uint8 slotIndex, int amount)
{
    WeaponSlotState* weaponState = GetWeaponSlotByIndex(slotIndex);
    if (!weaponState)
        return false;
    
    if (weaponState->runtime.ammoInMag < amount)
        return false;
    
    weaponState->runtime.ammoInMag -= amount;
    return true;
}

bool PlayerCombatComponent::CanFireWeapon(uint8 slotIndex) const
{
    const WeaponSlotState* weaponState = GetWeaponSlotByIndex(slotIndex);
    if (!weaponState)
        return false;
    
    if (weaponState->runtime.weaponId == 0)
        return false;
    
    if (weaponState->runtime.isReloading)
        return false;
    
    return weaponState->runtime.ammoInMag > 0;
}
