#pragma once
#include "Data/Tables/ZoneTable.h"

struct ServerConfig;

/*-------------------
   GameDataManager
-------------------*/
//
// GameDataManager는 게임에서 사용되는 다양한 데이터들을 관리하는 클래스입니다.
//

class GameDataManager
{
public:
   bool Init(const ServerConfig& config);
   const ZoneTable& GetZoneTable() const { return zoneTable_; }
   
private:
   ZoneTable zoneTable_;
   
};
