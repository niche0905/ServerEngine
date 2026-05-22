#include "pch.h"
#include "DropSystem.h"
#include <random>
#include "Content/Object/Actor/Actor.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Content/Object/Actor/PlayerPawn.h"
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
   
   SE::Math::Vector3 RandomScatterPosition(
    const SE::Math::Vector3& origin,
    float minRadius,
    float maxRadius)
   {
      static thread_local std::mt19937 rng(std::random_device{}());
      std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

      if (maxRadius < minRadius)
         std::swap(minRadius, maxRadius);

      minRadius = std::max(0.0f, minRadius);

      const float u = dist01(rng);
      const float angle = dist01(rng) * 2.0f * Pi;

      // 도넛 영역의 면적 기준 균등 분포
      const float minR2 = minRadius * minRadius;
      const float maxR2 = maxRadius * maxRadius;
      const float distance = std::sqrt(minR2 + u * (maxR2 - minR2));

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

void DropSystem::OnEntityDied(ObjectId entityId)
{
   Pawn* pawn = ownerRoom_->GetObjectManager().FindAs<Pawn>(entityId);
   if (!pawn)
      return;
   
   DropSpawnContext ctx{};
   ctx.reason = DropReason::Death;
   ctx.owner = entityId;

   switch (pawn->GetObjectType())
   {
   case ObjectType::OBJ_PLAYER:
      {
         PlayerPawn* playerPawn = static_cast<PlayerPawn*>(pawn);
         auto& inventory = playerPawn->GetInventory();
         const auto& slots = inventory.GetSlots();
         
         for (const auto& slot : slots) {
            if (!slot.IsValid())
               continue;   // 유효하지 않은 아이템 스택은 무시
            
            ctx.lootBundle.items.push_back(slot);
         }
      }
      break;
      
   case ObjectType::OBJ_MONSTER:
      {
         MonsterPawn* monsterPawn = static_cast<MonsterPawn*>(pawn);
         ctx.lootBundle = monsterPawn->GenerateDrops();
      }
      break;
      
   default:
      break;
   }
   
   DropItems(ctx);
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
            if (!itemStack.IsValid())
               continue;   // 유효하지 않은 아이템 스택은 무시
            
            SpawnWorldItemParams spawnParams;
            spawnParams.itemStack = itemStack;
            spawnParams.position = RandomScatterPosition(actorPos, 100, 200);       // TEMP: 반경 200 유닛 내에서 흩뿌리기
                                                                                             // TODO: 이 값도 Config 값으로 뺴던가 하기
            spawnParams.position.z = actorPos.z + 50.f;                                      // TEMP: 아이템이 땅에 묻히지 않도록 Actor의 높이보다 약간 위에서 생성하기 (50 유닛)
            spawnParams.initialVelocity = SE::Math::Vector3{ 0, 0, 0 };   // TEMP: 초기 속도는 0으로 (나중에 랜덤한 초기 속도 주던가 하기)
            spawnParams.reason = ctx.reason;
            
            auto* worldItem = ownerRoom_->SpawnItem(spawnParams);
            // TODO: 초기에 처리해 주어야 할 값이 있다면 추가로 처리하기
            
            result.spawned = true;
            result.spawnedCount += itemStack.count;
         }
         
         return result;
      }
      break;
   case DropMode::CorpseBox:
      // TODO: 만약 시체 박스를 생성하고 동기화 할 생각이라면 구현하기 (지금은 생각 없음)
      consoleLogger->Log(Color::Yellow, L"[DropSystem] You Don't have CorpseBox Spawn Build\n");
      break;
   }
   
   return DropSpawnResult{};
}
