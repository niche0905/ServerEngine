#include "pch.h"
#include "CombatSystem.h"

#include "ProjectileSweepQuery.h"
#include "Content/Object/ObjectId.h"
#include "Content/Object/Actor/Pawn.h"
#include "Content/Object/Actor/ProjectileActor.h"
#include "Content/Object/Actor/StaticActor.h"
#include "Content/Object/Actor/SubProjectile/RocketActor.h"
#include "Data/Map/ServerMap.h"
#include "Physics/Ray/RaycastHit.h"
#include "Service/Room/Room.h"

/*----------------
   CombatSystem
----------------*/

bool CombatSystem::Init(Room* ownerRoom, const ServerMap& mapData)
{
   if (!ownerRoom)
      return false;
   
   ownerRoom_ = ownerRoom;
   mapData_ = &mapData;
   
   return true;
}

bool CombatSystem::TraceHit(const SE::Physics::Ray& ray, ObjectId exceptId, SE::Physics::Hit::HitResult& outHit) const
{
   if (ownerRoom_ == nullptr or mapData_ == nullptr)
      return false;
   
   bool hasHit = false;
   float closestT = std::numeric_limits<float>::max();

   SE::Physics::RaycastHit raycastResult;
   if (mapData_->Raycast(ray, raycastResult)) {
      if (raycastResult.hit and raycastResult.t < closestT) {
         outHit.hit = raycastResult.hit;
         outHit.t = raycastResult.t;
         outHit.point = raycastResult.point;
         outHit.normal = raycastResult.normal;
         
         outHit.group = SE::Physics::Hit::HitGroup::NotHurtBox;
         outHit.damageMultiplier = 0.0f;
         outHit.partIndex = 0;
         outHit.actor = nullptr;
         
         closestT = raycastResult.t;
         hasHit = true;
      }
   }
   
   ownerRoom_->GetObjectManager().ForEachAlive([&](BaseObject* obj)
   {
      if (!obj)
         return;
      
      if (obj->GetId() == exceptId)
         return;   // 제외할 오브젝트는 건너뛰기
      
      obj->ForEachCollider([&](ColliderComponent* collider)
      {
         if (!collider)
            return;
         
         const ColliderRole role = collider->GetRole();
         if (role != ColliderRole::Hurtbox)
            return;   // 명중 판정이 필요한 콜라이더가 아닌 경우 건너 뛰기
         
         SE::Physics::RaycastHit rayHit{};
         if (!collider->GetCollider()->Raycast(ray, rayHit)) 
            return;
         
         if (!rayHit.hit)
            return;
         
         if (rayHit.t >= closestT) 
            return;
         
         outHit.hit = rayHit.hit;
         outHit.t = rayHit.t;
         outHit.point = rayHit.point;
         outHit.normal = rayHit.normal;
         
         outHit.group = SE::Physics::Hit::HitGroup::Torso;
         outHit.damageMultiplier = 1.0f;   // TODO: HitGroup에 따른 데미지 배율 적용하기
         outHit.partIndex = 0;   // TODO: HitGroup에 따른 부위 인덱스 적용하기
         outHit.actor = collider->GetOwnerActor();
         
         closestT = rayHit.t;
         hasHit = true;
      });
   });
   
   return hasHit;
}

bool CombatSystem::LaunchRocket(const SE::Math::Vector3& pos, const SE::Math::Vector3& dir, Pawn* ownerPawn,
   int32 damage, float speed, uint32 lifetimeMs, float radius)
{
   if (!ownerPawn)
      return false;   // 유효하지 않은 발사체 소유자 Pawn
   
   RocketActor* rocket = ownerRoom_->SpawnObject<RocketActor>(ObjectFlags::Replicable | ObjectFlags::Tickable);
   if (!rocket)
      return false;  // 발사체 생성 실패
   
   rocket->Init(ownerPawn->GetId(), pos, dir * speed, damage, lifetimeMs, 15.0f, radius, true);   // TEMP: 폭탄 충돌체의 반경은 15cm 정도로
   ownerRoom_->NotifyProjectileSpawn(rocket, /*templateId=*/0);  // TODO: templateId는 나중에 생각하기
   return true;
}

bool CombatSystem::SweepProjectile(const ProjectileSweepQuery& query, SE::Physics::Hit::HitResult& outHit) const
{
   if (ownerRoom_ == nullptr or mapData_ == nullptr)
      return false;
   
   const SE::Math::Vector3 delta = query.to - query.from;
   const float dist = delta.Length();

   if (dist <= 0.001f)
      return false;

   float closestT = std::numeric_limits<float>::max();
   bool hasHit = false;
   
   auto TryPickClosest = [&](const SE::Physics::RaycastHit& sweepHit,
                          SE::Physics::Hit::HitGroup group,
                          float damageMultiplier,
                          Actor* actor)
   {
      if (!sweepHit.hit)
         return;

      if (sweepHit.t < 0.0f || sweepHit.t > dist)
         return;

      if (sweepHit.t >= closestT)
         return;

      outHit.hit = true;
      outHit.t = sweepHit.t;
      outHit.point = sweepHit.point;
      outHit.normal = sweepHit.normal;

      outHit.group = group;
      outHit.damageMultiplier = damageMultiplier;
      outHit.partIndex = 0;
      outHit.actor = actor;

      closestT = sweepHit.t;
      hasHit = true;
   };

   if (query.hitMap) {
      SE::Physics::RaycastHit mapHit{};
      if (mapData_->SphereCast(query.from, query.to, query.radius, mapHit)) {
         TryPickClosest(
             mapHit,
             SE::Physics::Hit::HitGroup::NotHurtBox,
             0.0f,
             nullptr
         );
      }
   }
   
   ownerRoom_->GetObjectManager().ForEachAlive([&](BaseObject* obj)
   {
      if (!obj)
         return;

      if (obj->GetId() == query.projectileId)
         return;
      
      if (obj->GetId() == query.ownerId)
         return;   // 발사체 자신과 발사체의 소유자는 명중 판정에서 제외하기
      
      obj->ForEachCollider([&](ColliderComponent* collider)
      {
         if (!collider || !collider->GetCollider())
            return;

         const ColliderRole role = collider->GetRole();

         if (role == ColliderRole::Block) {
            if (!query.hitBlockActor)
               return;
         }
         else if (role == ColliderRole::Hurtbox) {
            if (!query.hitHurtBox)
               return;
         }
         else {
            return;
         }

         SE::Physics::RaycastHit actorHit{};
         if (!collider->GetCollider()->SphereCast(query.from, query.to, query.radius, actorHit)) {
            return;
         }

         if (role == ColliderRole::Block) {
            TryPickClosest(actorHit, SE::Physics::Hit::HitGroup::NotHurtBox, 0.0f, collider->GetOwnerActor());
         }
         else if (role == ColliderRole::Hurtbox) {
            TryPickClosest(actorHit, SE::Physics::Hit::HitGroup::Torso, 1.0f, collider->GetOwnerActor());
         }
      });
   });
   
   return hasHit;
}

void CombatSystem::ProjectileExplosion(ObjectId projectileId, const SE::Math::Vector3& pos, ObjectId ownerId,
                                       int32 damage, float radius, bool distanceDamageEnabled)
{
   if (projectileId == ObjectId{}) {
      consoleLogger->Log(Color::Yellow, L"[CombatSystem] ProjectileExplosion called with invalid projectileId\n");
      return;
   }
   
   if (ownerId == ObjectId{}) {
      consoleLogger->Log(Color::Yellow, L"[CombatSystem] ProjectileExplosion called with invalid ownerId\n");
   }
   
   PlayerId playerId = 0;
   Pawn* pawn = ownerRoom_->GetObjectManager().FindAs<Pawn>(ownerId);
   if (pawn) {
      playerId = pawn->GetOwnerPlayerId();
   } else {
      consoleLogger->Log(Color::Yellow, L"[CombatSystem] ProjectileExplosion: Owner Pawn not found for ownerId %u\n", ownerId.value);
   }
   
   const float radiusSq = radius * radius;
   
   ownerRoom_->GetObjectManager().ForEachAlive([&](BaseObject* obj)
   {
      if (!obj)
         return;
      
      Pawn* targetPawn = dynamic_cast<Pawn*>(obj);
      if (!targetPawn)
         return;   // 폭발 데미지를 적용할 수 있는 Pawn이 아닌 경우 건너뛰기
      
      const SE::Math::Vector3 targetPos = targetPawn->GetPosition();
      // TODO: Pos를 GetPosition이 아닌 가상 함수로 추가하는 편이 좋을 수 있음

      SE::Math::Vector3 toTarget = targetPos - pos;
      const float distSq = toTarget.LengthSq();
      
      if (distSq > radiusSq)
         return;   // 폭발 범위 밖에 있는 경우 건너뛰기
      
      const float dist = std::sqrt(distSq);
      
      SE::Physics::Ray losRay;
      losRay.origin = pos;
      losRay.direction = toTarget.Normalized();
      
      if (IsExplosionBlocked(losRay, dist, targetPawn->GetId()))
         return;
      
      int32 finalDamage = damage;
      
      if (distanceDamageEnabled) {
         const float t = std::clamp(dist / radius, 0.0f, 1.0f);
         
         const float multiplier = 1.0f - t;
         
         finalDamage = static_cast<int32>(std::round(static_cast<float>(damage) * multiplier));
      }
      
      if (finalDamage <= 0)
         return;   // 최종 데미지가 0 이하인 경우 건너뛰기
      
      DamageContext ctx;
      ctx.attacker = projectileId;
      ctx.instigator = ownerId;
      ctx.type = DamageType::Explosion;
      ctx.source = DamageSource::Weapon;
      
      DamageResult damageResult = targetPawn->ApplyDamage(ownerRoom_->GetObjectManager(), finalDamage, ctx);
   });
   
   ownerRoom_->NotifyExplosion(projectileId, playerId, pos, radius);
}

bool CombatSystem::IsExplosionBlocked(const SE::Physics::Ray& ray, float dist, ObjectId targetId) const
{
   constexpr float EPSILON = 1.0f;
   
   {
      SE::Physics::RaycastHit mapHit{};
      if (mapData_->Raycast(ray, mapHit)) {
         if (mapHit.hit and mapHit.t < dist - EPSILON) {
            return true;
         }
      }
   }
   
   bool blocked = false;
   
   ownerRoom_->GetObjectManager().ForEachAlive([&](BaseObject* obj)
   {
      if (blocked)
         return;
      
      if (!obj)
         return;
      
      if (obj->GetId() == targetId)
         return;
      
      StaticActor* staticActor = dynamic_cast<StaticActor*>(obj);
      if (!staticActor)
         return;
      
      staticActor->ForEachCollider([&](ColliderComponent* collider)
      {
         if (blocked)
            return;
         
         if (!collider or !collider->GetCollider())
            return;
         
         const ColliderRole role = collider->GetRole();
         
         if (role != ColliderRole::Block)
            return;   // 폭발 차단 판정이 필요한 콜라이더가 아닌 경우 건너 뛰기
         
         SE::Physics::RaycastHit rayHit{};
         if (!collider->GetCollider()->Raycast(ray, rayHit))
            return;
         
         if (!rayHit.hit)
            return;
         
         if (rayHit.t > 0.0f and rayHit.t < dist - EPSILON) 
            blocked = true;
      });
   });
   
   return blocked;
}
