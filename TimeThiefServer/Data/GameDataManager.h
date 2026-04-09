#pragma once
#include "Data/Tables/ZoneTable.h"

/*-------------------
   GameDataManager
-------------------*/
//
// GameDataManager는 게임에서 사용되는 다양한 데이터들을 관리하는 클래스입니다.
//

class GameDataManager
{
public:
   bool Init();
   const ZoneTable& GetZoneTable() const { return zoneTable_; }
   
private:
   ZoneTable zoneTable_;
   
};
