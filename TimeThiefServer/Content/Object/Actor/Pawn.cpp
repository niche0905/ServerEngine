#include "pch.h"
#include "Pawn.h"
#include "Content/Gameplay/Combat/CombatComponent.h"
#include "Service/Room/Room.h"

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
   // TODO: 체력이 변한 게 Player Pawn이라면 해당 클라이언트에게 체력 업데이트 패킷 보내기 (혹은 Room에서 BroadcastHealthUpdate(...))
   // room->BroadcastHealthUpdate(...);
   
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
   
   // 사망 시 모든 Client에 사망 사실 Broadcast (Room에서 BroadcastDeath 패킷을 보내는 형태로)
   // 파생 클래스에서 필요에 따라 재정의 (DropOnDeath, RespawnSchedule 등)
   if (auto room = GetRoom()) {
      room->BroadcastDeath(GetId());
   }
}
