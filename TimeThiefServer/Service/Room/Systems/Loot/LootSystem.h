#pragma once
#include "Content/Gameplay/Loot/LootTypes.h"

struct LootTable;
class Room;

/*---------------
   LootSystem
---------------*/
//
// LootSystem는 게임 내에서 드롭 테이블을 참조하여 적절한 아이템을 생성하는 시스템입니다.
//

class LootSystem
{
public:
   LootSystem() = default;
   
   bool Init(Room* ownerRoom, const LootTable& lootTable);
   
   LootBundle GenerateLootBundle(int32 tableId) const;
   LootBundle GenerateLootBundle(int32 tableId, uint32 rngSeed) const;
   
private:
   Room* ownerRoom_;
   const LootTable* lootTable_;
   
};
