#include "pch.h"
#include "UpgradeComponent.h"
#include "Content/Gameplay/Economy/StoreTypes.h"
#include "Content/Object/Actor/PlayerPawn.h"

/*--------------------
   UpgradeComponent
--------------------*/

void UpgradeComponent::Init(BaseObject* owner)
{
   SetOwner(owner);
   Clear();
   
   statUpgradeLevels_[Health_S] = 0;
   statUpgradeLevels_[Speed_S] = 0;
   InitStats();
}

void UpgradeComponent::InitStats()
{
   InitStatUpgrade(Health_S);
   InitStatUpgrade(Speed_S);
}

void UpgradeComponent::InitStatUpgrade(StatUpgradeCode code)
{
   if (code == 0)
      return;
   
   int32 level = statUpgradeLevels_[code];
   
   if (PlayerPawn* player = GetOwnerAs<PlayerPawn>()) {
      player->OnStatUpgradeApplied(code, level);
   }
}

int32 UpgradeComponent::GetUpgradeLineLevel(uint32 lineId) const
{
   if (appliedUpgradeEntryLevels_.contains(lineId)) {
      return appliedUpgradeEntryLevels_.at(lineId);
   }
   
   return 0;
}

bool UpgradeComponent::ApplyUpgradeLine(uint32 lineId, int32 maxLevel, int32 deltaLevel)
{
   if (lineId == 0 || deltaLevel <= 0)
      return false;
   
   int32& level = appliedUpgradeEntryLevels_[lineId];
   int32 oldLevel = level;
   level = std::min(level + deltaLevel, maxLevel);
   
   if (oldLevel == level)
      return false;   // 이미 최대 레벨이어서 적용할 수 없는 경우
   
   return true;
}

bool UpgradeComponent::ApplyUpgrade(uint32 lineId, int32 maxLevel, uint32 templateId)
{
   if (lineId == 0 and maxLevel <= 0) {
      return ApplyWeaponUpgrade(templateId);
   }
   else {
      ApplyUpgradeLine(lineId, maxLevel, 1);
      int32 newLevel = GetUpgradeLineLevel(lineId);
      return ApplyStatUpgrade(templateId, maxLevel, newLevel);
   }
}

bool UpgradeComponent::CanApplyUpgrade(uint32 lineId, int32 maxLevel) const
{
   if (maxLevel <= 0)
      return false;
   
   if (appliedUpgradeEntryLevels_.contains(lineId)) {
      int32 currentLevel = appliedUpgradeEntryLevels_.at(lineId);
      if (currentLevel >= maxLevel)
         return false;   // 이미 최대 레벨이어서 적용할 수 없는 경우
   }
   
   return true;
}

bool UpgradeComponent::HasWeaponUpgrade(WeaponUpgradeCode code) const
{
   if (code == 0)
      return false;
   
   return weaponUpgradeCodes_.contains(code);
}

bool UpgradeComponent::CanApplyWeaponUpgrade(WeaponUpgradeCode code) const
{
   if (code == 0)
      return false;
   
   return !HasWeaponUpgrade(code);
}

bool UpgradeComponent::ApplyWeaponUpgrade(WeaponUpgradeCode code)
{
   if (!CanApplyWeaponUpgrade(code))
      return false;
   
   weaponUpgradeCodes_.insert(code);
   
   if (PlayerPawn* player = GetOwnerAs<PlayerPawn>()) {
      player->OnWeaponUpgradeApplied(code);
   }
   
   return true;
}

int32 UpgradeComponent::GetStatUpgradeLevel(StatUpgradeCode code) const
{
   if (code == 0)
      return 0;
   
   auto it = statUpgradeLevels_.find(code);
   if (it == statUpgradeLevels_.end())
      return 0;
   
   return it->second;
}

bool UpgradeComponent::CanApplyStatUpgrade(StatUpgradeCode code, int32 maxLevel) const
{
   if (code == 0 or maxLevel <= 0)
      return false;
   
   return GetStatUpgradeLevel(code) < maxLevel;
}

bool UpgradeComponent::ApplyStatUpgrade(StatUpgradeCode code, int32 maxLevel, int32 newLevel)
{
   if (code == 0)
      return false;
   
   if (!CanApplyStatUpgrade(code, maxLevel))
      return false;
   
   int32& level = statUpgradeLevels_[code];
   int32 oldLevel = level;
   level = std::min(newLevel, maxLevel);
   
   if (oldLevel == level)
      return false;   // 이미 최대 레벨이어서 적용할 수 없는 경우
   
   if (PlayerPawn* player = GetOwnerAs<PlayerPawn>()) {
      player->OnStatUpgradeApplied(code, level);
   }
   
   return true;
}

void UpgradeComponent::Clear()
{
   weaponUpgradeCodes_.clear();
   statUpgradeLevels_.clear();
   appliedUpgradeEntryLevels_.clear();
}

UpgradeSnapshot UpgradeComponent::CaptureSnapshot() const
{
   UpgradeSnapshot result;
   result.weaponUpgradeCodes = weaponUpgradeCodes_;
   result.statUpgradeLevels = statUpgradeLevels_;
   result.appliedUpgradeEntryLevels_ = appliedUpgradeEntryLevels_;
   return result;
}

void UpgradeComponent::RestoreSnapshot(const UpgradeSnapshot& snapshot)
{
   
   appliedUpgradeEntryLevels_ = snapshot.appliedUpgradeEntryLevels_;
   statUpgradeLevels_ = snapshot.statUpgradeLevels;
   weaponUpgradeCodes_ = snapshot.weaponUpgradeCodes;
   
   // 현재 상태에 맞게 플레이어의 스탯과 무기 스탯 새로고침하기
   InitStats();
   if (PlayerPawn* player = GetOwnerAs<PlayerPawn>()) {
      player->RefreshWeaponStats();
   }
}
