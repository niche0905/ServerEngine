#include "pch.h"
#include "MonsterPawn.h"

#include "Service/Room/Room.h"

/*---------------
   MonsterPawn
---------------*/

LootSourceResult MonsterPawn::GenerateLoot(ObjectManager& om, LootTableService& service, const LootSourceContext& ctx)
{
   return loot_.GenerateLoot(om, service, ctx);
}

LootBundle MonsterPawn::GenerateDrops()
{
   // TODO: Loot (확률에 의한 아이템 드롭) 반환하기
   //       지금 코드에서 문제가 없는지 검토 필요
   
   if (not loot_.CanGenerateLoot())
      return LootBundle{};
   
   auto room = GetRoom();
   if (room == nullptr)
      return LootBundle{};

   // TODO: LootTableService에서 몬스터의 LootTable을 찾아서 드랍 생성하기 Loot Service 연동 필요...
   return LootBundle{};
}

void MonsterPawn::UpdateAI(ObjectManager& om, float dt)
{
   if (IsDead())
      return;
   
   // TODO: BTBrain 작성 후 연동
   (void)om;
   (void)dt;
}

void MonsterPawn::OnSpawn()
{
   Pawn::OnSpawn();

   respawn_.Init(GetId(), RespawnPolicy{});
   loot_.Init(GetId(), 0);
   // TODO: AI 컴포넌트 초기화 (BT 트리 로드)
   // ai_.Initialize(this, objectManager_, "path...");
   
   SetDead(false);
}

void MonsterPawn::Tick(float dt)
{
   if (IsDead())
      return;

   Pawn::Tick(dt);
   
   ai_.Tick(dt);
   
}

void MonsterPawn::OnPreDestroy()
{
   ai_.Shutdown();
   Pawn::OnPreDestroy();
   // 필요하다면 정리
   // ex) brain detach, 드랍 정리 등
}

void MonsterPawn::OnDeath(ObjectManager& om, const DamageResult& dmgResult)
{
   (void)dmgResult;
   // 죽음 상태로 전환
   // 리스폰 처리
   // respawn_.OnDeath(nowMs);      
   // 아니면 드랍을 여기서 처리할 수도 있음 (om을 전달받아야 함)
}

void MonsterPawn::StartDeadState(ObjectManager& om, const DamageResult& dmgResult)
{
   // 죽음 처리
   SetVelocity(Vector3{}); // 정지
   
   // TODO: Service를 받아오고, ctx 채우기... 여러가지 정보를 더 얻어와야 함
   // GenerateLoot(om, service, ctx);
   
   // TODO: om에 GetTimeMs 필요 or 인자로 nowMs 전달 필요
   // ScheduleRespawn(om, om.GetTimeMs());
}

// void MonsterPawn::ScheduleRespawn(ObjectManager& om, uint64 nowMs)
// {
//    (void)om;
//    
//    if (not CanRespawn())
//       return;
//    
//    respawn_.Schedule(nowMs, GetRespawnDelayMs());
// }
//
// void MonsterPawn::ExecuteRespawn(ObjectManager& om, uint64 nowMs)
// {
//    respawn_.ExecuteRespawn(om, *this, RespawnContext{ nowMs } );
// }
