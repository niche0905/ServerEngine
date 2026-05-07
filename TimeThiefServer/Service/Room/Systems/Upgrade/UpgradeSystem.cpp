#include "pch.h"
#include "UpgradeSystem.h"

/*-----------------
   UpgradeSystem
-----------------*/

bool UpgradeSystem::Init(Room* ownerRoom, const UpgradeTable& table)
{
   if (!ownerRoom)
      return false;
   
   ownerRoom_ = ownerRoom;
   upgradeTable_ = &table;
   
   return true;
}

const UpgradeTable* UpgradeSystem::GetUpgradeTable() const
{
   return upgradeTable_;
}

int32 UpgradeSystem::GetStatDelta(StatUpgradeCode code, int32 currentLevel) const
{
   if (auto* statUpgradeInfo = upgradeTable_->StatUpgradeTable.Find(code)) {
      if (auto* currentLevelInfo = statUpgradeInfo->GetLevelByStat(currentLevel)) {
         if (currentLevelInfo->statCode != code) {
            consoleLogger->Log(Color::Yellow, L"Not Same Stat Code\n");
         }
         
         return currentLevelInfo->statDelta;
      }
   }
   
   return 0;   // 유효하지 않은 코드이거나, 현재 레벨에 대한 정보가 없는 경우
}

int32 UpgradeSystem::GetStatFinalValue(StatUpgradeCode code, int32 currentLevel) const
{
   int32 result = 0;
   // TODO: 기본 값을 가져올 수 있어야 함 (StatCode에 따른 기본 Start Stat)
   
   if (auto* statUpgradeInfo = upgradeTable_->StatUpgradeTable.Find(code)) {
      for (int32 level = 1; level <= currentLevel; ++level) {
         if (auto* levelInfo = statUpgradeInfo->GetLevelByStat(level)) {
            if (levelInfo->statCode != code) {
               consoleLogger->Log(Color::Yellow, L"Not Same Stat Code\n");
            }
            
            // 누적된 델타를 더해나감
            result += levelInfo->statDelta;
         }
      }
   }
   
   return result;
}
