#pragma once
#include "Content/Gameplay/Loot/LootTypes.h"
#include "Content/Object/ObjectId.h"

struct LootSourceContext
{
   ObjectId owner;      // 드랍 주체
   ObjectId killer;     // 드랍 처치자
   uint64 nowMs{0};     // 현재 시간 (밀리초) <- 서버 시간 기준
   uint32 rngSeed{0};  // 드랍 시드 (재현 가능하게)
   LootRollContext roll;   // 드랍 롤 컨텍스트 (레벨 보정 등)
};

struct LootSourceResult
{
   bool generated{false};
   LootBundle bundle;
};
