#include "pch.h"
#include "MonsterPawn.h"

/*---------------
   MonsterPawn
---------------*/

Actor::Vector3 MonsterPawn::ResolveRespawnPosition(ObjectManager& om)
{
   (void)om;
   
   return GetHomePosition();
}

void MonsterPawn::OnPreRespawn(ObjectManager& om)
{
   (void)om;
   // TODO: 리스폰 전 처리 (예: 상태 초기화)
}

void MonsterPawn::OnPostRespawn(ObjectManager& om)
{
   (void)om;
   // TODO: 리스폰 후 처리 (예: AI 재활성화)
}

void MonsterPawn::ApplyRespawnToWorld(ObjectManager& om, const Vector3& pos)
{
   // TODO: Room에 리스폰 알리기
}

void MonsterPawn::GrantSpawnInvulnerability(ObjectManager& om, uint32 durationMs)
{
   health_.SetInvincible(true);
   // TODO: 일정 시간 후에 invincible 해제하는 로직 추가
   (void)om;
   (void)durationMs;
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
   drop_.Init(GetId(), DropOnDeathPolicy{});
   
   SetDead(false);
}

void MonsterPawn::Tick(float dt)
{
   Pawn::Tick(dt);
   
   if (IsDead())
      return;
}

void MonsterPawn::OnPreDestroy()
{
   Pawn::OnPreDestroy();
   // 필요하다면 정리
   // ex) brain detach, 드랍 정리 등
}

void MonsterPawn::OnDeath(const DamageResult& dmgResult)
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
   
   // TODO: Drop 시스템 개편 후 처리
   drop_.Generate(om, *this, DropOnDeathContext{});
   
   // TODO: om에 GetTimeMs 필요 or 인자로 nowMs 전달 필요
   // ScheduleRespawn(om, om.GetTimeMs());
}

void MonsterPawn::ScheduleRespawn(ObjectManager& om, uint64 nowMs)
{
   (void)om;
   
   if (not CanRespawn())
      return;
   
   respawn_.Schedule(nowMs, GetRespawnDelayMs());
}

void MonsterPawn::ExecuteRespawn(ObjectManager& om, uint64 nowMs)
{
   respawn_.ExecuteRespawn(om, *this, RespawnContext{ nowMs } );
}
