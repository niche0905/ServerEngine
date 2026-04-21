#pragma once

class PlayerPawn;
struct WeaponStat;
struct WeaponTable;
class Room;

/*----------------
   WeaponSystem
----------------*/
//
// WeaponSystem는 플레이어의 무기 Stat을 Dtat table을 이용하여 설정하는 시스템 입니다
//

class WeaponSystem
{
public:
   WeaponSystem() = default;
   
   bool Init(Room* ownerRoom, const WeaponTable& weaponTable);
   
   const WeaponStat* GetBaseWeaponStat(uint32 weaponId) const;
   
   bool BuildFinalWeaponStat(PlayerPawn* player, uint32 weaponId, WeaponStat& outStat) const;
   
   bool RebuildWeapon(PlayerPawn* player, int slotIndex);
   bool RebuildAllWeapons(PlayerPawn* player);
   
private:
   Room*                      ownerRoom_ = nullptr;    // non-owning
   const WeaponTable*         weaponTable_ = nullptr;
    
};
