#include "pch.h"
#include "LootSystem.h"
#include "Service/Room/Room.h"
#include "Data/Tables/LootTableTypes.h"

/*---------------
   LootSystem
---------------*/

bool LootSystem::Init(Room* ownerRoom, const LootTable& lootTable)
{
   if (ownerRoom == nullptr)
      return false;
   
   ownerRoom_ = ownerRoom;
   lootTable_ = &lootTable;
   
   return true;
}

LootBundle LootSystem::GenerateLootBundle(int32 tableId) const
{
   Random32& rng = ownerRoom_->GetRandom();
   return GenerateLootBundle(tableId, rng.NextU32());
}

LootBundle LootSystem::GenerateLootBundle(int32 tableId, uint32 rngSeed) const
{
   return lootTable_->Roll(tableId, rngSeed);
}
