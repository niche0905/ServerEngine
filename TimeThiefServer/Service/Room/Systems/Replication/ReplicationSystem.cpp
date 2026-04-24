#include "pch.h"
#include "ReplicationSystem.h"
#include <algorithm>
#include <chrono>
#include "Content/Gameplay/Replication/IObjectReplicator.h"
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
   ev.header.type = RepEventType::Spawn;
   ev.header.source = object->GetId();
   replicationEvents_.push_back(ev);
   
   MarkDirty(object->GetId());
}

void ReplicationSystem::NotifyDespawn(ObjectId objectId)
{
   if (!ownerRoom_)
      return;   // 유효하지 않은 ownerRoom
   
   RepEvent ev;
   ev.header.type = RepEventType::Despawn;
   ev.header.source = objectId;
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
      
      if (ev.header.tick == 0)
         ev.header.tick = frame.roomTick;
      
      if (ev.header.timeMs == 0) {
         ev.header.timeMs = static_cast<uint64>(std::chrono::duration_cast<Milliseconds>(frame.now.time_since_epoch()).count());
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
      
      auto* replicator = GetReplicator(obj->GetObjectType());
      if (!replicator) {
         consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] No replicator for object type {}, skipping replication for objectId {}",
                            obj->GetObjectType(), obj->GetId().value);
         continue;   // 해당 오브젝트 타입에 대한 Replicator가 없는 경우 (예: 아직 구현되지 않았거나, 복제할 필요가 없는 타입인 경우)에는 스킵
      }
      
      auto result = replicator->FlushPeriodic(obj, repState.GetFlags(), frame, nowMs, *ownerRoom_);
      
      if (result.sent) {
         repState.lastReplicatedTick = frame.roomTick;
         repState.lastReplicatedTimeMs = nowMs;
         ++repState.replicationVersion;
         
         repState.ClearDirty(result.handled);
         
      }

      if (repState.IsDirty()) {
         nextDirtyObjects.push_back(objectId);
         nextDirtySet.insert(objectId);
      }
   }
   
   dirtyObjects_.swap(nextDirtyObjects);
   dirtyObjectSet_.swap(nextDirtySet);
}

const IObjectReplicator* ReplicationSystem::GetReplicator(ObjectType objectType)
{
   switch (objectType)
   {
   case ObjectType::OBJ_PLAYER:
      return &playerReplicator_;
      
   case ObjectType::OBJ_MONSTER:
      return &monsterReplicator_;
      
   case ObjectType::OBJ_PROJECTILE:
      return &projectileReplicator_;
      
   default:
      return nullptr;
   }
}
