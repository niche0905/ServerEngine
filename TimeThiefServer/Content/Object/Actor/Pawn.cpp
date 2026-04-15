#include "pch.h"
#include "Pawn.h"
#include "Data/Tables/ZoneTableJson.h"
#include "Service/Room/Room.h"

/*--------
   Pawn
--------*/

void Pawn::SetVelocity(const Vector3& velocity)
{
   velocity_ = velocity;
   // 필요 시 속도 변경에 따른 추가 작업 수행
}

void Pawn::IntegrateMove(float dt)
{
   Vector3 pos = GetPosition();
   pos.x += velocity_.x * dt;
   pos.y += velocity_.y * dt;
   pos.z += velocity_.z * dt;

   SetPosition(pos);
}

DamageResult Pawn::ApplyDamage(ObjectManager& om, int32 amount, const DamageContext& ctx)
{
   if (IsDead()) {
      DamageResult result;
      result.requested = amount;
      return result;
   }
   
   amount = ResolveIncomingDamage(amount, ctx);
   OnBeforeApplyDamage(ctx, amount);
   
   if (amount <= 0) {
      DamageResult result;
      result.requested = amount;
      result.applied = 0;
      result.accepted = false;
      
      return result;
   }
   
   DamageResult result = health_.ApplyDamage(amount, ctx);
   if (result.accepted) {
      Damaged(result);
      OnAfterApplyDamage(result, ctx);
      
      if (not health_.IsAlive() and not isDead_) {
         isDead_ = true;
         OnDeath(om, result);
      
         if (ShouldRequestDestroyOnDeath())
         {
            om.RequestDestroy(GetId());
         }
      }
   }
   
   return result;
}

void Pawn::Damaged(const DamageResult& dmgResult)
{
   // None
}

bool Pawn::IsHpAlive() const
{
   return health_.IsAlive();
}

int32 Pawn::GetHp() const
{
   return health_.GetHp();
}

int32 Pawn::GetMaxHp() const
{
   return health_.GetMaxHp();
}

SE::Math::Vector3 Pawn::ResolveRespawnPosition(ObjectManager& om)
{
   return GetSavedRespawnPosition();
}

void Pawn::OnPreRespawn(ObjectManager& om)
{
   (void)om;
   // TODO: 리스폰 전 처리 (예: 상태 초기화)
   isDead_ = false;
   int32 maxHp = health_.GetMaxHp();
   health_.Revive(maxHp);
}

void Pawn::OnPostRespawn(ObjectManager& om)
{
   (void)om;
   // TODO: 리스폰 후 처리 할 게 있는지 확인
}

void Pawn::ApplyRespawnToWorld(ObjectManager& om, const SE::Math::Vector3& pos)
{
   (void)om;
   (void)pos;
   
   SetDead(false);
   
   if (auto room = GetRoom()) {
      room->HandlePawnRespawn(GetId());
   }
}

void Pawn::GrantSpawnInvulnerability(ObjectManager& om, uint32 durationMs)
{
   health_.SetInvincible(true);
   // TODO: 일정 시간 후에 invincible 해제하는 로직 추가
   (void)om;
   (void)durationMs;
}

void Pawn::OnSpawn()
{
   Actor::OnSpawn();
   
   health_.Init(GetId(), 100);
   cooldowns_.Init(GetId());
   effects_.Init(GetId());
   
   isDead_ = false;
   velocity_ = Vector3{};
}

void Pawn::OnPreDestroy()
{
   Actor::OnPreDestroy();
   // 추가 정리 작업이 필요할 경우 여기에 작성
   // ex) 전투 종료, AI 중단, 루팅 생성 등
}

void Pawn::Tick(float dt)
{
   IntegrateMove(dt);
}

void Pawn::OnDeath(ObjectManager& om, const DamageResult& dmgResult)
{
   (void)om;
   (void)dmgResult;
   
   respawn_.MarkDead();
   // 사망 시 모든 Client에 사망 사실 Broadcast (Room에서 BroadcastDeath 패킷을 보내는 형태로)
   // 파생 클래스에서 필요에 따라 재정의 (DropOnDeath, RespawnSchedule 등)
   if (auto room = GetRoom()) {
      room->HandlePawnDeath(GetId(), dmgResult);
   }
}
