#include "pch.h"
#include "MonsterPawn.h"
#include "PlayerPawn.h"
#include "Data/GameDataManager.h"
#include "Physics/Collider/CapsuleCollider.h"
#include "Service/Room/Room.h"
#include "Data/AI/AiBlackboard.h"

namespace BB = AiBlackboardKey;

namespace
{
   constexpr float MaxAwareness = 100.0f;
   constexpr float SuspiciousThreshold = 20.0f;
   constexpr float AlertedThreshold = 55.0f;
   constexpr float CombatThreshold = 80.0f;
   constexpr float AwarenessDecayPerSecond = 8.0f;
   constexpr float HateDecayPerSecond = 2.0f;
   constexpr float DamageHateMultiplier = 2.0f;
   constexpr float TargetSwitchHateMargin = 25.0f;
}

/*---------------
   MonsterPawn
---------------*/

void MonsterPawn::SetTarget(Pawn* pawn)
{
   const ObjectId newTargetId = (pawn) ? pawn->GetId() : ObjectId{};
   
   if (ai_.GetTargetId() == newTargetId) {
      return;
   }
   
   ai_.SetTargetId(newTargetId);
   MarkReplicationDirty(ReplicationDirty::Target);
   RefreshAlertLevel();
}

void MonsterPawn::ClearTarget()
{
   ai_.ClearTarget();
   MarkReplicationDirty(ReplicationDirty::Target);
   RefreshAlertLevel();
}

float MonsterPawn::GetHate(ObjectId targetId) const
{
   const auto it = hateByTarget_.find(targetId);
   return (it != hateByTarget_.end()) ? it->second : 0.0f;
}

void MonsterPawn::AddHate(ObjectId targetId, float amount)
{
   if (targetId == ObjectId{} || amount <= 0.0f)
      return;

   hateByTarget_[targetId] += amount;
}

void MonsterPawn::ReceiveNoiseStimulus(ObjectId sourceId, const Vector3& position, float loudness)
{
   if (IsDead() || loudness <= 0.0f)
      return;

   lastStimulusPosition_ = position;
   hasStimulusPosition_ = true;
   alertValue_ = std::clamp(alertValue_ + loudness, 0.0f, MaxAwareness);
   AddHate(sourceId, loudness * 0.5f);
   RefreshAlertLevel();

   if (GetTargetId() == ObjectId{} && !hasMovePath_)
      LookAtPosition(position);
}

Pawn* MonsterPawn::SelectTarget(float acquireRange, const std::function<bool(Pawn*)>& predicate) const
{
   auto room = GetRoom();
   if (!room || acquireRange <= 0.0f)
      return nullptr;

   Pawn* bestTarget = nullptr;
   float bestScore = -std::numeric_limits<float>::infinity();
   const float acquireRangeSq = acquireRange * acquireRange;
   const Vector3 selfPos = GetPosition();

   room->GetObjectManager().ForEachAlive([&](BaseObject* obj)
   {
      Pawn* pawn = dynamic_cast<Pawn*>(obj);
      if (!pawn || pawn->IsDead() || pawn->GetObjectType() != ObjectType::OBJ_PLAYER)
         return;
      if (predicate && !predicate(pawn))
         return;

      Vector3 diff = pawn->GetPosition() - selfPos;
      diff.z = 0.0f;
      const float distSq = diff.LengthSq();
      if (distSq > acquireRangeSq)
         return;

      // Hate가 우선권을 주되, 같은 Hate에서는 가까운 대상을 선택한다.
      const float distanceRatio = std::sqrt(distSq) / acquireRange;
      const float score = GetHate(pawn->GetId()) - distanceRatio * 10.0f;
      if (score > bestScore) {
         bestScore = score;
         bestTarget = pawn;
      }
   });

   return bestTarget;
}

DamageResult MonsterPawn::ApplyDamage(ObjectManager& om, int32 amount, const DamageContext& ctx)
{
   DamageResult pawnApplyResult = Pawn::ApplyDamage(om, amount, ctx);

   const ObjectId threatId = (ctx.instigator != ObjectId{}) ? ctx.instigator : ctx.attacker;
   if (pawnApplyResult.accepted && threatId != ObjectId{}) {
      AddHate(threatId, static_cast<float>(pawnApplyResult.applied) * DamageHateMultiplier);
      alertValue_ = MaxAwareness;
      RefreshAlertLevel();
   }
   
   if (threatId != ObjectId{}) {
      PlayerPawn* attacker = om.FindAs<PlayerPawn>(threatId);
      
      if (attacker) {
         if (auto blackboard = ai_.GetBlackboard()) {
            const ObjectId currentTargetId = blackboard->get<ObjectId>(BB::TargetId);
            if (currentTargetId != ObjectId{}
                && GetHate(threatId) < GetHate(currentTargetId) + TargetSwitchHateMargin)
               return pawnApplyResult;
            
            blackboard->set<Pawn*>(BB::TargetPawn, attacker);
            blackboard->set<ObjectId>(BB::TargetId, ctx.attacker);
            
            SetTarget(attacker);
         }
      }
   }

   return pawnApplyResult;
}

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

   RespawnPolicy respawnPolicy{};
   respawnPolicy.invulMs = 1000;

   int32 lootTableId = 1;

   if (auto room = GetRoom()) {
      if (auto* gameDataManager = room->GetGameDataManager()) {
         if (const MonsterTemplateDef* monsterTemplate = gameDataManager->GetMonsterTemplateTable().GetTemplate(templateId_)) {
            health_.Init(this, monsterTemplate->maxHp);
            respawnPolicy.enabled = monsterTemplate->respawnTimeSec > 0;
            if (respawnPolicy.enabled) {
               respawnPolicy.delayMs = static_cast<uint32>(monsterTemplate->respawnTimeSec) * 1000;
            }
            lootTableId = monsterTemplate->lootTableId;
            dropPoint_ = monsterTemplate->dropPoint;
            BindHitboxProfile(monsterTemplate->collisionProfileId);
         }
      }
   }

   respawn_.Init(this, respawnPolicy);
   loot_.Init(this, lootTableId);

   if (auto room = GetRoom()) {
      ai_.Initialize(this, &room->GetObjectManager(), templateId_);
   }

   switch (templateId_)
   {
   case 4:     // boss gorilla
      {
         auto bodyCollider = std::make_unique<ColliderComponent>();
         auto capsuleCollider = std::make_unique<SE::Physics::CapsuleCollider>(Vector3{0.0f, 0.0f, 160.0f}, Vector3{0.0f, 0.0f, 240.0f}, 160.0f);
         bodyCollider->Init(this, ColliderRole::Hurtbox, std::move(capsuleCollider));

         colliders_.push_back(std::move(bodyCollider));
      }
      break;

   case 3:     // minion
      {
         auto bodyCollider = std::make_unique<ColliderComponent>();
         auto capsuleCollider = std::make_unique<SE::Physics::CapsuleCollider>(Vector3{0.0f, 0.0f, 44.0f}, Vector3{0.0f, 0.0f, 116.0f}, 44.0f);
         bodyCollider->Init(this, ColliderRole::Hurtbox, std::move(capsuleCollider));

         colliders_.push_back(std::move(bodyCollider));
      }
      break;
      
   case 2:     // cat
      {
         auto bodyCollider = std::make_unique<ColliderComponent>();
         auto capsuleCollider = std::make_unique<SE::Physics::CapsuleCollider>(Vector3{-36.0f, 0.0f, 60.0f}, Vector3{36.0f,  0.0f, 60.0f},  44.0f);
         bodyCollider->Init(this, ColliderRole::Hurtbox, std::move(capsuleCollider));
         
         colliders_.push_back(std::move(bodyCollider));
      }
      break;
   }
   
   SetDead(false);
}

void MonsterPawn::Tick(float dt)
{
   if (IsDead())
      return;

   // Pawn::Tick(dt);
   
   ai_.Tick(dt);
   UpdateAwareness(dt);
   
   UpdateMove(dt);
}

void MonsterPawn::UpdateAwareness(float dt)
{
   if (dt <= 0.0f)
      return;

   if (GetTargetId() == ObjectId{})
      alertValue_ = std::max(0.0f, alertValue_ - AwarenessDecayPerSecond * dt);
   else
      alertValue_ = MaxAwareness;

   for (auto it = hateByTarget_.begin(); it != hateByTarget_.end(); ) {
      it->second -= HateDecayPerSecond * dt;
      if (it->second <= 0.0f)
         it = hateByTarget_.erase(it);
      else
         ++it;
   }

   RefreshAlertLevel();
}

void MonsterPawn::RefreshAlertLevel()
{
   if (GetTargetId() != ObjectId{} || alertValue_ >= CombatThreshold)
      alertLevel_ = MonsterAlertLevel::Combat;
   else if (alertValue_ >= AlertedThreshold)
      alertLevel_ = MonsterAlertLevel::Alerted;
   else if (alertValue_ >= SuspiciousThreshold)
      alertLevel_ = MonsterAlertLevel::Suspicious;
   else
      alertLevel_ = MonsterAlertLevel::Calm;
}

void MonsterPawn::ResetAwareness()
{
   hateByTarget_.clear();
   alertValue_ = 0.0f;
   alertLevel_ = MonsterAlertLevel::Calm;
   lastStimulusPosition_ = Vector3{};
   hasStimulusPosition_ = false;
}

void MonsterPawn::OnPreDestroy()
{
   ai_.Shutdown();
   Pawn::OnPreDestroy();
   // 필요하다면 정리
   // ex) brain detach, 드랍 정리 등
}

void MonsterPawn::OnPreRespawn(ObjectManager& om)
{
   Pawn::OnPreRespawn(om);
   
   ai_.Stop();
   ai_.HaltTree();
   ai_.ResetBlackboard();

   StopMove();
   ResetAwareness();
}

void MonsterPawn::OnPostRespawn(ObjectManager& om)
{
   Pawn::OnPostRespawn(om);
   
   ai_.Start();
}

void MonsterPawn::MoveTo(const Vector3& targetPos, float acceptRadius)
{
   moveAcceptRadius_ = acceptRadius;
   finalMoveTarget_ = targetPos;

   auto room = GetRoom();
   if (room == nullptr) {
      StopMove();
      return;
   }

   auto* navQueryContext = room->GetNavigationQueryContext();
   if (navQueryContext == nullptr) {
      StopMove();
      return;
   }

   const ServerMap& map = room->GetGameDataManager()->GetServerMap();

   std::vector<Vector3> path;
   if (map.FindPath(*navQueryContext, GetPosition(), targetPos, path) != NavPathResult::Success || path.empty() ) {
      StopMove();
      return;
   }

   MoveAlongPath(std::move(path), acceptRadius);
}

void MonsterPawn::MoveAlongPath(std::vector<Vector3> path, float acceptRadius)
{
   if (path.empty()) {
      StopMove();
      return;
   }

   movePath_ = std::move(path);
   movePathIndex_ = 0;

   // 마지막 지점을 최종 목표로 저장
   finalMoveTarget_ = movePath_.back();
   moveAcceptRadius_ = acceptRadius;

   hasMovePath_ = true;
}

void MonsterPawn::StopMove()
{
   hasMovePath_ = false;
   movePath_.clear();
   movePathIndex_ = 0;
   SetVelocity(Vector3{});
}

void MonsterPawn::UpdateMove(float dt)
{
   if (!hasMovePath_ || movePath_.empty())
      return;

   const Vector3 pos = GetPosition();

   const float finalDistSq = (finalMoveTarget_ - pos).LengthSq();
   if (finalDistSq <= moveAcceptRadius_ * moveAcceptRadius_) {
      StopMove();
      return;
   }

   while (movePathIndex_ < movePath_.size())
   {
      const Vector3 waypoint = movePath_[movePathIndex_];
      const float waypointDistSq = (waypoint - pos).LengthSq();

      if (waypointDistSq > waypointAcceptRadius_ * waypointAcceptRadius_)
         break;

      ++movePathIndex_;
   }

   if (movePathIndex_ >= movePath_.size()) {
      StopMove();
      return;
   }

   Vector3 next = movePath_[movePathIndex_];

   Vector3 toNext = next - pos;
   toNext.z = 0.0f;

   if (toNext.LengthSq() <= 0.0001f) {
      ++movePathIndex_;
      return;
   }

   const Vector3 dir = toNext.Normalized();
   const float moveDelta = moveSpeed_ * dt;

   const float dist = std::sqrt(toNext.LengthSq());

   if (moveDelta >= dist) {
      SetPosition(next);
      ++movePathIndex_;
   }
   else {
      SetPosition(pos + dir * moveDelta);
   }

   SetVelocity(dir * moveSpeed_);

   // 바라보는 방향도 갱신
   LookAtDirection(dir);
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
   
   StopMove();
   
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
