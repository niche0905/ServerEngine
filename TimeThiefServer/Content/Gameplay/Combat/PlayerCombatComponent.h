#pragma once
#include "CombatComponent.h"
#include "CombatTypes.h"
#include "Physics/Hitbox/HitResult.h"
#include "Physics/Ray/Ray.h"

/*-------------------------
   PlayerCombatComponent
-------------------------*/
//
// PlayerCombatComponent는 플레이어 Pawn의 전투 관련 기능을 구현하는 컴포넌트입니다.
// 사격과 무기 변경과 같은 플레이어의 공격 행동을 처리하는 기능을 포함할 예정입니다.
//

class PlayerCombatComponent : public CombatComponent
{
private:
   struct PalletPattern;
   
public:
   virtual ~PlayerCombatComponent() = default;
   
public:
   virtual void Init(BaseObject* owner) override;
   
   virtual bool CanAttack(const AttackRequest& request) const override;
   virtual bool TryReload();
   
   uint32 GetHandWeaponId() const;
   bool SwitchWeapon(uint32 newWeaponId);
   
   const WeaponSlotState* GetCurrentWeaponSlot() const;
   WeaponSlotState* GetCurrentWeaponSlot();
   
   const WeaponSlotState* GetWeaponSlot(uint32 weaponId) const;
   WeaponSlotState* GetWeaponSlot(uint32 weaponId);
   
   const WeaponSlotState* GetWeaponSlotByIndex(uint8 slotIndex) const;
   WeaponSlotState* GetWeaponSlotByIndex(uint8 slotIndex);
   
   uint8 GetCurrentWeaponIndex() const;
   uint32 GetCurrentWeaponId() const;
   
   size_t GetWeaponSlotCount() const { return combatState_.slots.size(); }
   
protected:
   virtual bool ExecuteAttack(AttackRequest& request) override;
   
private:
   bool TraceHit(const AttackRequest& request, const SE::Physics::Ray& ray, SE::Physics::Hit::HitResult& outHit);
   bool FireRifle(const AttackRequest& request);
   bool FireShotgun(const AttackRequest& request);
   void FireLauncher(const AttackRequest& request);
   PalletPattern GeneratePalletPattern(const SE::Math::Vector3& forwardDir, int palletCount, float spreadDegrees, uint32 shotSeed) const;
   
   bool IsValidWeaponSlot(uint8 slotIndex) const;
   bool ConsumeAmmo(uint8 slotIndex, int amount);
   bool CanFireWeapon(uint8 slotIndex) const;
   
private:
   struct PalletPattern
   {
      std::vector<SE::Math::Vector3> directions;
   };

private:
   PlayerCombatState combatState_{};
   
};
