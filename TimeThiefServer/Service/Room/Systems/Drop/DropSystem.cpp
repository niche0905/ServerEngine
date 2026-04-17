#include "pch.h"
#include "DropSystem.h"
#include <random>
#include "Content/Object/Actor/Actor.h"
#include "Service/Room/Room.h"

/*-----------------
   Local Helper
-----------------*/

namespace
{
   SE::Math::Vector3 RandomScatterPosition(const SE::Math::Vector3& origin, float radius)
   {
      static thread_local std::mt19937 rng(std::random_device{}());
      std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

      const float u = dist01(rng);
      const float angle = dist01(rng) * 2.0f * Pi;

      const float distance = radius * std::sqrt(u);

      const float offsetX = std::cos(angle) * distance;
      const float offsetY = std::sin(angle) * distance;

      return origin + SE::Math::Vector3{ offsetX, offsetY, 0.0f };
   }
}

/*---------------
   DropSystem
---------------*/

bool DropSystem::Init(Room* ownerRoom)
{
   if (!ownerRoom)
      return false;   // 유효하지 않은 ownerRoom
   
   ownerRoom_ = ownerRoom;
   
   return true;
}

DropSpawnResult DropSystem::DropItems(const DropSpawnContext& ctx)
{
   if (!ownerRoom_)
      return DropSpawnResult{};   // 시스템이 초기화되지 않은 경우, 드롭 실패로 간주
   
   switch (ctx.mode)
   {
   case DropMode::Scatter:
      {
         auto* actor = ownerRoom_->GetObjectManager().FindAs<Actor>(ctx.owner);
         if (!actor)
            break;
         
         const auto& actorPos = actor->GetPosition();
         DropSpawnResult result{};
         for (const auto& itemStack : ctx.lootBundle.items) {
            SpawnWorldItemParams spawnParams;
            spawnParams.itemStack = itemStack;
            spawnParams.position = RandomScatterPosition(actorPos, /*radius=*/200.0f);       // TEMP: 반경 200 유닛 내에서 흩뿌리기
                                                                                             // TODO: 이 값도 Config 값으로 뺴던가 하기
            spawnParams.position.z += actorPos.z + 50.0f;                                    // TEMP: 아이템이 땅에 묻히지 않도록 Actor의 높이보다 약간 위에서 생성하기 (50 유닛)
            spawnParams.initialVelocity = SE::Math::Vector3{ 0, 0, 0 };   // TEMP: 초기 속도는 0으로 (나중에 랜덤한 초기 속도 주던가 하기)
            spawnParams.reason = ctx.reason;
            
            auto* worldItem = ownerRoom_->SpawnItem(spawnParams);
            // TODO: 초기에 처리해 주어야 할 값이 있다면 추가로 처리하기
            
            result.spawned = true;
            result.spawnedCount += itemStack.count;
         }
         
         return result;
         // TODO: 돈의 경우 Loot Bundle에 포함되지 않는다 (처치 시 강탈하는 것으로)
         
      }
      break;
   case DropMode::CorpseBox:
      // TODO: 만약 시체 박스를 생성하고 동기화 할 생각이라면 구현하기 (지금은 생각 없음)
      consoleLogger->Log(Color::Yellow, L"[DropSystem] You Don't have CorpseBox Spawn Build\n");
      break;
   }
   
   return DropSpawnResult{};
}
