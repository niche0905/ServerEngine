#include "pch.h"
#include "Pawn.h"
#include "Data/Tables/ZoneTableJson.h"
#include "Service/Room/Room.h"
#include "Shard/GameShard.h"

/*--------
   Pawn
--------*/

void Pawn::IntegrateMove(float dt)
{
   Vector3 pos = GetPosition();
   pos.x += velocity_.x * dt;
   pos.y += velocity_.y * dt;
   pos.z += velocity_.z * dt;

   SetPosition(pos);
}

void Pawn::LookAtDirection(const Vector3& dir)
{
   if (dir.LengthSq() <= 0.0001f)
      return;

   const float yawRad = std::atan2(dir.y, dir.x);
   const float yawDeg = yawRad * (180.0f / Pi);

   SetYaw(yawDeg);
}

void Pawn::LookAtPosition(const Vector3& targetPos)
{
   Vector3 dir = targetPos - GetPosition();
   dir.z = 0.0f;

   LookAtDirection(dir);
}

DamageResult Pawn::ApplyDamage(ObjectManager& om, int32 amount, const DamageContext& ctx)
{
   if (IsDead()) {
      DamageResult result;
      result.requested = amount;
      return result;
   }
   
   amount = ResolveIncomingDamage(amount, ctx);
   
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
         OnDeath(om, ctx, result);
      
         if (ShouldRequestDestroyOnDeath())
         {
            if (auto room = GetRoom()) {
               room ->HandleDespawn(GetId());
            }
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
   auto room = GetRoom();
   if (!room)
      return;   // Room이 없는 경우, 유효하지 않은 상태
   
   auto* ownerShard = room->GetOwnerShard();
   if (!ownerShard)
      return;  // Room이 소속된 샤드가 없는 경우, 유효하지 않은 상태
   
   RoomId roomId = room->GetRoomId();
   ObjectId objectId = GetId();
   
   health_.SetInvincible(true);
   room->ScheduleAfter(Milliseconds(durationMs), [ownerShard, roomId, objectId]()
        {
            auto room = ownerShard->FindRoom(roomId);
            if (!room)
               return;   // Room이 없는 경우, 유효하지 않은 상태
      
            if (Pawn* pawn = room->GetObjectManager().FindAs<Pawn>(objectId)) {
               pawn->GetHealth().SetInvincible(false);
            }
       });
   
   (void)om;
   (void)durationMs;
}

bool Pawn::TryReserveRespawn()
{
   return true;
}

void Pawn::CancelReserveRespawn()
{
   respawn_.CancelScheduled();
}

void Pawn::OnSpawn()
{
   Actor::OnSpawn();
   
   health_.Init(this, 100);
   cooldowns_.Init(this);
   effects_.Init(this);
   
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

void Pawn::OnDeath(ObjectManager& om, const DamageContext& ctx, const DamageResult& dmgResult)
{
   (void)om;
   (void)dmgResult;
   
   lastKillerId_ = ctx.attacker;
   if (ctx.instigator != ObjectId{}) {
      lastKillerId_ = ctx.instigator;
   }
   
   respawn_.MarkDead();
   // 사망 시 모든 Client에 사망 사실 Broadcast (Room에서 BroadcastDeath 패킷을 보내는 형태로)
   // 파생 클래스에서 필요에 따라 재정의 (DropOnDeath, RespawnSchedule 등)
   if (auto room = GetRoom()) {
      room->HandlePawnDeath(this, ctx, dmgResult);
   }
}
