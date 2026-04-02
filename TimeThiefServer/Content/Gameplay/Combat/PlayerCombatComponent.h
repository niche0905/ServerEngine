#pragma once
#include "CombatComponent.h"
#include "CombatTypes.h"

/*-------------------------
   PlayerCombatComponent
-------------------------*/
//
// PlayerCombatComponent는 플레이어 Pawn의 전투 관련 기능을 구현하는 컴포넌트입니다.
// 사격과 무기 변경과 같은 플레이어의 공격 행동을 처리하는 기능을 포함할 예정입니다.
//

class PlayerCombatComponent : public CombatComponent
{
public:
   virtual ~PlayerCombatComponent() = default;
   
public:
   void Init(ObjectId owner, Pawn* ownerPawn);
   
   virtual bool CanAttack(const AttackRequest& request) const override;
   virtual bool TryReload();
   bool SwitchWeapon(uint32 newWeaponId);
   
   bool IsAiming() const;
   void SetAiming(bool aiming);
   
   const WeaponState* GetCurrentWeaponState() const;
   WeaponState* GetCurrentWeaponState();
   
   const WeaponState* GetWeaponState(uint32 weaponId) const;
   WeaponState* GetWeaponState(uint32 weaponId);
   
   const WeaponState* GetWeaponStateBySlot(uint8 slotIndex) const;
   WeaponState* GetWeaponStateBySlot(uint8 slotIndex);
   
   uint8 GetCurrentWeaponSlot() const;
   uint32 GetCurrentWeaponId() const;
   
protected:
   virtual bool ExecuteAttack(const AttackRequest& request) override;
   
private:
   bool FireHitscan(const AttackRequest& request);
   bool FireMelee(const AttackRequest& request);
   
   bool IsValidWeaponSlot(uint8 slotIndex) const;
   bool ConsumeAmmo(uint8 slotIndex, int amount);
   bool CanFireWeapon(uint8 slotIndex) const;

private:
   PlayerCombatState combatState_{};
   
};
