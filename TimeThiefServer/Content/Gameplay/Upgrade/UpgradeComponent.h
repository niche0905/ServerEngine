#pragma once
#include "Content/Shared/BaseComponent.h"
#include <unordered_set>
#include <unordered_map>
#include "TypesDef.h"

/*--------------------
   UpgradeComponent
--------------------*/
//
// UpgradeComponent는 플레이어가 게임 내에서 업그레이드를 획득하고 적용할 수 있도록 하는 컴포넌트입니다.
//

class UpgradeComponent : public BaseComponent
{
public:
   virtual void Init(BaseObject* owner);
   
   void InitStatUpgrade(StatUpgradeCode code);
   
public:
   int32 GetUpgradeLineLevel(uint32 lineId) const;
   bool ApplyUpgradeLine(uint32 lineId, int32 maxLevel, int32 deltaLevel = 1);
   
   bool ApplyUpgrade(uint32 lineId, int32 maxLevel, uint32 templateId);
   
   bool CanApplyUpgrade(uint32 lineId, int32 maxLevel) const;
   
// Weapon
public:
   bool HasWeaponUpgrade(WeaponUpgradeCode code) const;
   bool CanApplyWeaponUpgrade(WeaponUpgradeCode code) const;
   bool ApplyWeaponUpgrade(WeaponUpgradeCode code);
   
// Stat
public:
   int32 GetStatUpgradeLevel(StatUpgradeCode code) const;
   bool CanApplyStatUpgrade(StatUpgradeCode Code, int32 maxLevel) const;
   bool ApplyStatUpgrade(StatUpgradeCode code, int32 maxLevel, int32 newLevel);
   
// Util
public:
   void Clear();
   const std::unordered_set<WeaponUpgradeCode>& GetAllWeaponUpgrades() const { return weaponUpgradeCodes_; }
   
private:
   std::unordered_set<WeaponUpgradeCode>           weaponUpgradeCodes_;    // 플레이어가 획득한 무기 업그레이드 코드들의 집합
   std::unordered_map<StatUpgradeCode, int32>      statUpgradeLevels_;     // 플레이어가 획득한 스탯 업그레이드 코드와 해당 레벨의 맵
   
   std::unordered_map<uint32, int32>               appliedUpgradeEntryLevels_;   // 적용된 업그레이드 엔트리 ID와 해당 레벨의 맵 (업그레이드 적용 시, 중복 적용 방지 및 업그레이드 단계 관리용)
};
