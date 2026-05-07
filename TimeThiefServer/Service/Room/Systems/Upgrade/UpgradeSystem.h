#pragma once
#include "Data/Tables/UpgradeTable.h"

class Room;

/*-----------------
   UpgradeSystem
-----------------*/
//
// UpgradeSystem는 Stat에 관련한 Upgrade를 적용할 수 있게 하는 보조 시스템입니다
//

class UpgradeSystem
{
public:
   UpgradeSystem() = default;
   
   bool Init(Room* ownerRoom, const UpgradeTable& table);
   
   const UpgradeTable* GetUpgradeTable() const;
   int32 GetStatDelta(StatUpgradeCode code, int32 currentLevel) const;
   int32 GetStatFinalValue(StatUpgradeCode code, int32 currentLevel) const;
   
private:
   Room*                      ownerRoom_ = nullptr;   // non-owning
   const UpgradeTable*        upgradeTable_ = nullptr;
    
};
