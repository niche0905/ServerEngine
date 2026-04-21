#include "pch.h"
#include "UpgradeComponent.h"

#include "Content/Object/Actor/PlayerPawn.h"

/*--------------------
   UpgradeComponent
--------------------*/

void UpgradeComponent::Init(BaseObject* owner)
{
   SetOwner(owner);
   Clear();
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
   
   // TODO: 업그레이드 적용에 따른 추가 로직 (예: 플레이어 능력치 변경, 새로운 스킬 활성화 등)
   //       매번 참조하는 것은 branch stall도 그렇고 비효율 적일 수 있다 (Weapon Stat에 대한 정보를 Player마다 일단 들고 있으면 효율적일 듯?)
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

bool UpgradeComponent::ApplyStatUpgrade(StatUpgradeCode code, int32 maxLevel, int32 deltaLevel)
{
   if (code == 0 || deltaLevel <= 0)
      return false;
   
   if (!CanApplyStatUpgrade(code, maxLevel))
      return false;
   
   int32& level = statUpgradeLevels_[code];
   level = std::min(level + deltaLevel, maxLevel);
   
   // TODO: 업그레이드 적용에 따른 추가 로직 (예: 플레이어 능력치, 혹은 Skill에 쿨타임 변경 등 적용)
   if (PlayerPawn* player = GetOwnerAs<PlayerPawn>()) {
      player->OnStatUpgradeApplied(code, level);
   }
   
   return true;
}

void UpgradeComponent::Clear()
{
   weaponUpgradeCodes_.clear();
   statUpgradeLevels_.clear();
}
