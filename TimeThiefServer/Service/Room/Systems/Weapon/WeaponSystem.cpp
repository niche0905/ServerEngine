#include "pch.h"
#include "WeaponSystem.h"

#include "Content/Gameplay/Upgrade/UpgradeComponent.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Data/Tables/WeaponTable.h"

/*----------------
   WeaponSystem
----------------*/

bool WeaponSystem::Init(Room* ownerRoom, const WeaponTable& weaponTable)
{
   if (!ownerRoom)
      return false;   // 유효하지 않은 ownerRoom
   
   ownerRoom_ = ownerRoom;
   weaponTable_ = &weaponTable;
   return true;
}

const WeaponStat* WeaponSystem::GetBaseWeaponStat(uint32 weaponId) const
{
   return weaponTable_ ? weaponTable_->GetWeaponStat(weaponId) : nullptr;
}

bool WeaponSystem::BuildFinalWeaponStat(PlayerPawn*player, uint32 weaponId, WeaponStat& outStat) const
{
   const WeaponStat* base = GetBaseWeaponStat(weaponId);
   if (!base or !player)
      return false;
   
   outStat = *base;
   
   const UpgradeComponent& upgrade = player->GetUpgrade();
   
   // TODO: 작성하기 (우선 Upgrade table이 필요할 듯 싶음)   
}

bool WeaponSystem::RebuildWeapon(PlayerPawn* player, int slotIndex)
{
}

bool WeaponSystem::RebuildAllWeapons(PlayerPawn* player)
{
}
