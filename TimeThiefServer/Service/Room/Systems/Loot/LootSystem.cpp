#include "pch.h"
#include "LootSystem.h"

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

LootBundle LootSystem::GenerateLootBundle(int32 lootTableId, uint32 rngSeed) const
{
   return lootTable_->Roll(lootTableId, rngSeed);
}
