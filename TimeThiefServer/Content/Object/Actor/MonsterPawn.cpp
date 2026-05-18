#include "pch.h"
#include "MonsterPawn.h"
#include "Data/GameDataManager.h"
#include "Service/Room/Room.h"

/*---------------
   MonsterPawn
---------------*/

LootBundle MonsterPawn::GenerateDrops()
{
   auto room = GetRoom();
   if (room == nullptr)
      return LootBundle{};

   return room->GetRoomGameSystem().GetLootSystem().GenerateLootBundle(loot_.GetTableId());
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

   respawn_.Init(this, RespawnPolicy{});
   loot_.Init(this, 1);    // TEMP: LootSourceComponent의 tableId는 1로 고정
   // TODO: AI 컴포넌트 초기화 (BT 트리 로드)
   if (auto room = GetRoom()) {
      ai_.Initialize(this, &room->GetObjectManager(), 1);
   }
   
   SetDead(false);
}

void MonsterPawn::Tick(float dt)
{
   if (IsDead())
      return;

   // Pawn::Tick(dt);
   
   ai_.Tick(dt);
   
   UpdateMove(dt);
}

void MonsterPawn::OnPreDestroy()
{
   ai_.Shutdown();
   Pawn::OnPreDestroy();
   // 필요하다면 정리
   // ex) brain detach, 드랍 정리 등
}

void MonsterPawn::MoveTo(const Vector3& targetPos)
{
   moveTarget_ = targetPos;
   hasMovetarget_ = true;
}

void MonsterPawn::StopMove()
{
   hasMovetarget_ = false;
   SetVelocity(Vector3{});
}

void MonsterPawn::UpdateMove(float dt)
{
   if (!hasMovetarget_)
      return;
   
   const Vector3 pos = GetPosition();
   const Vector3 toTarget = moveTarget_ - pos;
   
   const float distSq = toTarget.LengthSq();
   if (distSq <= moveAcceptRadius_ * moveAcceptRadius_) {
      StopMove();
      return;
   }
   
   const float dist = std::sqrt(distSq);
   const Vector3 dir = toTarget.Normalized();
   
   const float moveDelta = moveSpeed_ * dt;
   
   if (moveDelta >= dist) {
      SetPosition(moveTarget_);
      StopMove();
      return;
   }
   
   SetPosition(pos + dir * moveDelta);
   SetVelocity(dir * moveDelta);
}

void MonsterPawn::StartAI()
{
   ai_.Start();
}

void MonsterPawn::StopAI()
{
   ai_.Stop();
}

void MonsterPawn::OnDeath(ObjectManager& om, const DamageContext& ctx, const DamageResult& dmgResult)
{
   Pawn::OnDeath(om, ctx, dmgResult);
   
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
