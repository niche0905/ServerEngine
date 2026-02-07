#pragma once
#include "LootSourceTypes.h"

class LootTableService;
class ObjectManager;

/*---------------
   ILootSource
---------------*/
//
// ILootSource는 게임 내에서 아이템을 드롭할 수 있는 모든 객체가 구현해야 하는 인터페이스입니다.
//

class ILootSource
{
public:
   virtual ~ILootSource() = default;
   
   // 지금 이 LootSource가 아이템을 드롭할 수 있는지 여부를 반환합니다.
   virtual bool CanGenerateLoot() const = 0;
   
   // 이 LootSource에서 아이템을 드롭합니다.
   virtual LootSourceResult GenerateLoot(ObjectManager& om, LootTableService& service, const LootSourceContext& ctx) = 0;
   
};
