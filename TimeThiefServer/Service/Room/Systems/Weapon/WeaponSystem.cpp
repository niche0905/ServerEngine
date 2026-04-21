#include "pch.h"
#include "WeaponSystem.h"

#include "Content/Gameplay/Upgrade/UpgradeComponent.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Data/Tables/WeaponTable.h"

/*----------------
   WeaponSystem
----------------*/

bool WeaponSystem::Init(Room* ownerRoom, const WeaponTable& weaponTable, const UpgradeTable& upgradeTable)
{
   if (!ownerRoom)
      return false;   // 유효하지 않은 ownerRoom
   
   ownerRoom_ = ownerRoom;
   weaponTable_ = &weaponTable;
   upgradeTable_ = &upgradeTable;
   return true;
}

const WeaponStat* WeaponSystem::GetBaseWeaponStat(uint32 weaponId) const
{
   return weaponTable_ ? weaponTable_->GetWeaponStat(weaponId) : nullptr;
}

bool WeaponSystem::CreateInitialWeaponSlot(PlayerPawn* player, uint32 weaponId, WeaponSlotState& outSlot) const
{
   if (!player or !weaponTable_)
      return false;
   
   WeaponStat finalStat;
   if (!BuildFinalWeaponStat(player, weaponId, finalStat))
      return false;
   
   outSlot.runtime.weaponId = weaponId;
   outSlot.runtime.ammoInMag = finalStat.common.magCapacity;
   outSlot.runtime.isReloading = false;
   
   outSlot.stat = finalStat;
   outSlot.dirty = false;
   
   return true;
}

bool WeaponSystem::BuildFinalWeaponStat(PlayerPawn* player, uint32 weaponId, WeaponStat& outStat) const
{
   const WeaponStat* base = GetBaseWeaponStat(weaponId);
   if (!base or !player)
      return false;
   
   outStat = *base;
   
   const UpgradeComponent& upgrade = player->GetUpgrade();
   const auto& upgradeCodes = upgrade.GetAllWeaponUpgrades();
   
   for (WeaponUpgradeCode code : upgradeCodes) {
      const auto* upgradeDef = upgradeTable_->WeaponUpgradeTable.Find(code);
      if (!upgradeDef)
         continue;
      
      if (upgradeDef->target.weaponId != weaponId)
         continue;
      
      const WeaponStatModifier& mod = upgradeDef->modifier;
      
      outStat.common.damage += mod.damageDelta;
      outStat.common.magCapacity += mod.magCapacityDelta;
      outStat.common.fireIntervalSec += mod.fireIntervalSecDelta;
      outStat.common.reloadTimeSec += mod.reloadTimeSecDelta;
      outStat.common.range += mod.rangeDelta;
      
      switch (outStat.common.category)
      {
      case WeaponCategory::Rifle:
         {
            
         }
         break;
      case WeaponCategory::Shotgun:
         {
            ShotgunStat* shotgun = std::get_if<ShotgunStat>(&outStat.extra);
            if (shotgun == nullptr) {
               consoleLogger->Log(Color::Red, L"WeaponSystem::BuildFinalWeaponStat: Shotgun weapon has no ShotgunStat in extra");
               return false;
            }
         
            shotgun->pelletCount += mod.palletCountDelta;
            shotgun->coneAngleDegrees += mod.coneAngleDegreesDelta;
         }
         break;
      case WeaponCategory::Launcher:
         {
            LauncherStat* launcher = std::get_if<LauncherStat>(&outStat.extra);
            if (launcher == nullptr) {
               consoleLogger->Log(Color::Red, L"WeaponSystem::BuildFinalWeaponStat: Launcher weapon has no LauncherStat in extra");
               return false;
            }
         
            launcher->projectileSpeed += mod.projectileSpeedDelta;
            launcher->explosionRadius += mod.explosionRadiusDelta;
         }
         break;
      }
   }
   
   return true;
}

bool WeaponSystem::RebuildWeapon(PlayerPawn* player, int slotIndex)
{
   if (!player)
      return false;
   
   auto* combat = player->GetPlayerCombat();
   if (!combat)
      return false;
   
   WeaponSlotState* slot = combat->GetWeaponSlotByIndex(slotIndex);
   if (!slot)
      return false;
   
   WeaponStat finalStat;
   if (!BuildFinalWeaponStat(player, slot->runtime.weaponId, finalStat))
      return false;
   
   slot->stat = finalStat;
   slot->dirty = false;
   
   return true;
}

bool WeaponSystem::RebuildAllWeapons(PlayerPawn* player)
{
   if (!player)
      return false;
   
   auto* combat = player->GetPlayerCombat();
   if (!combat)
      return false;
   
   const size_t slotCount = combat->GetWeaponSlotCount();
   
   bool allOk = true;
   for (size_t i = 0; i < slotCount; ++i) {
      if (!RebuildWeapon(player, static_cast<int>(i))) {
         consoleLogger->Log(Color::Red, L"WeaponSystem::RebuildAllWeapons: Failed to rebuild weapon in slot %d", i);
         allOk = false;
      }
   }
   
   return allOk;
}
