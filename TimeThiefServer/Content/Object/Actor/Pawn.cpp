#include "pch.h"
#include "Pawn.h"
#include "Content/Gameplay/Combat/CombatComponent.h"

/*--------
   Pawn
--------*/

void Pawn::SetVelocity(const Vector3& velocity)
{
   velocity_ = velocity;
   // 필요 시 속도 변경에 따른 추가 작업 수행
}

void Pawn::IntergrateMove(float dt)
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
   
   DamageResult result = health_.ApplyDamage(amount, ctx);
   
   if (not health_.IsAlive() and not isDead_) {
      isDead_ = true;
      OnDeath(om, result);
      
      if (ShouldRequestDestroyOnDeath())
      {
         __RequestDestroy();
      }
   }
   
   return result;
}

bool Pawn::IsAlive() const
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

void Pawn::OnSpawn()
{
   Actor::OnSpawn();
   
   health_.Init(GetId(), 100);
   cooldowns_.Init(GetId());
   effects_.Init(GetId());
   combat_->Init(GetId(), this);
   
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
   IntergrateMove(dt);
}

void Pawn::OnDeath(ObjectManager& om, const DamageResult& dmgResult)
{
   (void)om;
   (void)dmgResult;
   // 기본적으로 아무 작업도 하지 않음
   // 파생 클래스에서 필요에 따라 재정의 (DropOnDeath, RespawnSchedule 등)
}
