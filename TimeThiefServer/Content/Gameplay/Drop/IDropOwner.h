#pragma once
#include "Content/Gameplay/Loot/LootTypes.h"

/*--------------
   IDropOwner
--------------*/
//
// IDropOwner는 드롭 시스템에서 드롭 아이템을 생성하는 주체를 나타내는 인터페이스입니다.
//

class IDropOwner
{
public:
   virtual ~IDropOwner() = default;
   
   virtual LootBundle GenerateDrops() = 0;
   
};
