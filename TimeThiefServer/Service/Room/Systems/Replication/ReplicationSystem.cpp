#include "pch.h"
#include "ReplicationSystem.h"
#include <algorithm>
#include <chrono>
#include "Content/Gameplay/Replication/ReplicationEvent.h"
#include "Content/Object/BaseObject.h"
#include "Content/Object/ObjectManager.h"
#include "Content/Object/Actor/Pawn.h"
#include "Content/Object/Actor/ProjectileActor.h"
#include "Service/Room/Room.h"

/*---------------------
   ReplicationSystem
---------------------*/

bool ReplicationSystem::Init(Room* ownerRoom)
{
   if (!ownerRoom)
      return false;   // 유효하지 않은 ownerRoom
   
   ownerRoom_ = ownerRoom;
   dirtyObjects_.clear();
   dirtyObjectSet_.clear();
   replicationEvents_.clear();
   
   return true;
}

void ReplicationSystem::NotifySpawn(BaseObject* object)
{
   if (!ownerRoom_ or !object)
      return;   // 유효하지 않은 ownerRoom 또는 object
   
   RepEvent ev;
   ev.type = RepEventType::Spawn;
   ev.source = object->GetId();
   replicationEvents_.push_back(ev);
   
   MarkDirty(object->GetId());
}

void ReplicationSystem::NotifyDespawn(ObjectId objectId)
{
   if (!ownerRoom_)
      return;   // 유효하지 않은 ownerRoom
   
   RepEvent ev;
   ev.type = RepEventType::Despawn;
   ev.source = objectId;
   replicationEvents_.push_back(ev);
   
   dirtyObjectSet_.erase(objectId);
   std::erase(dirtyObjects_, objectId);
}

void ReplicationSystem::PushEvent(const RepEvent& event)
{
   if (!ownerRoom_)
      return;   // 유효하지 않은 ownerRoom
   
   replicationEvents_.push_back(event);
}

void ReplicationSystem::MarkDirty(ObjectId objectId)
{
   if (!ownerRoom_)
      return;
   
   const auto [it, inserted] = dirtyObjectSet_.insert(objectId);
   if (inserted) {
      dirtyObjects_.push_back(objectId);
   }
}

void ReplicationSystem::FlushImmediate(const RepFrame& frame)
{
   if (!ownerRoom_)
      return;   // 유효하지 않은 ownerRoom
   
   if (replicationEvents_.empty())
      return;
   
   // TODO:
   // 1. replicationEvents_를 순회하며 타입별 패킷 생성
   // 2. 관심 영역 / 시야 대상 필터링
   // 3. Broadcast 또는 개별 Send 수행
   //
   
   for (RepEvent& ev : replicationEvents_) {
      
      if (ev.tick == 0)
         ev.tick = frame.roomTick;
      
      if (ev.timeMs == 0) {
         ev.timeMs = static_cast<uint64>(std::chrono::duration_cast<Milliseconds>(frame.now.time_since_epoch()).count());
      }
      
      // ex)
      // switch (ev.type)
   }
   
   replicationEvents_.clear();
}

void ReplicationSystem::FlushPeriodic(const RepFrame& frame)
{
   if (!ownerRoom_)
      return;
   
   if (dirtyObjects_.empty())
      return;
   
   const uint64 nowMs = std::chrono::duration_cast<Milliseconds>(frame.now.time_since_epoch()).count();
   
   std::vector<ObjectId> nextDirtyObjects;
   nextDirtyObjects.reserve(dirtyObjects_.size());
   
   std::unordered_set<ObjectId> nextDirtySet;
   nextDirtySet.reserve(dirtyObjectSet_.size());
   
   for (const ObjectId objectId : dirtyObjects_)
   {
      BaseObject* obj = ownerRoom_->GetObjectManager().Find(objectId);
      if (!obj)
         continue;
      
      if (!obj->IsActive())
         continue;
      
      ReplicatedState& repState = obj->GetReplicatedState();
      if (!repState.IsDirty())
         continue;
      
      // TODO:
      // 여기서 repState.GetFlags()를 보고
      // Transform / Velocity / Health 등의 패킷을 생성해서 전송한다.
      //
      // 예:
      // if (repState.Has(ReplicationDirty::Transform)) { ... }
      // if (repState.Has(ReplicationDirty::Velocity))  { ... }
      bool sent = false;
      
      if (auto* pawn = dynamic_cast<Pawn*>(obj)) {
      }
      else if (auto* projectile = dynamic_cast<ProjectileActor*>(obj)) {
         sent = FlushProjectilePeriodic(projectile, repState.GetFlags(), frame, nowMs);
      }
      
      if (sent) {
         repState.lastReplicatedTick = frame.roomTick;
         repState.lastReplicatedTimeMs = nowMs;
         ++repState.replicationVersion;
         repState.ClearDirty();
         
         // 아직 dirty가 남아있다면 다음 periodic flush에서도 유지
         if (repState.IsDirty())
         {
            nextDirtyObjects.push_back(objectId);
            nextDirtySet.insert(objectId);
         }
      }
      else {
         nextDirtyObjects.push_back(objectId);
         nextDirtySet.insert(objectId);
      }
   }
   
   dirtyObjects_.swap(nextDirtyObjects);
   dirtyObjectSet_.swap(nextDirtySet);
}

bool ReplicationSystem::FlushProjectilePeriodic(ProjectileActor* projectile, ReplicationDirty flags, const RepFrame& frame, uint64 nowTimeMs)
{
   if (!projectile)
      return false;
   
   if (!(HasDirty(flags, ReplicationDirty::Transform) || HasDirty(flags, ReplicationDirty::Velocity)))
      return false;   // Transform이나 Velocity가 변경되지 않았다면 패킷 생성할 필요 없음
   
   if (nowTimeMs < projectile->GetReplicatedState().lastReplicatedTimeMs + 100) {   // TODO: 100ms이거 상수나 config 값으로 빼기
      return false;
   }
   
   se::game::N_ProjectileMove projectileMoveNoti;
   {
      auto* entityIdPtr = projectileMoveNoti.mutable_projectile_id();
      entityIdPtr->set_value(projectile->GetId().value);
      
      auto* positionPtr = projectileMoveNoti.mutable_position();
      const auto& pos = projectile->GetPosition();
      positionPtr->set_x(pos.x);
      positionPtr->set_y(pos.y);
      positionPtr->set_z(pos.z);
      
      auto* directionPtr = projectileMoveNoti.mutable_velocity();
      const auto& velocity = projectile->GetVelocity();
      directionPtr->set_x(velocity.x);
      directionPtr->set_y(velocity.y);
      directionPtr->set_z(velocity.z);
   }
   
   SendBufferRef moveNotiBuffer = ServerPacketHandler::MakeSendBuffer(projectileMoveNoti);
   if (!moveNotiBuffer)
      return false;
   
   ownerRoom_->BroadcastReplication(moveNotiBuffer);
   return true;
}
