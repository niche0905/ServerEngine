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
   
   const uint64 nowMs = std::chrono::duration_cast<Milliseconds>(frame.now.time_since_epoch()).count();
   
   for (RepEvent& ev : replicationEvents_) {
      
      NormalizeEvent(ev, frame, nowMs);
      DispatchImmediateEvent(ev, frame);
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
         consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] No replicator for object type %u, skipping replication for objectId %u\n",
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

void ReplicationSystem::NormalizeEvent(RepEvent& ev, const RepFrame& frame, uint64 nowMs) const
{
   if (ev.header.tick == 0) {
      ev.header.tick = frame.roomTick;
   }
   if (ev.header.timeMs == 0) {
      ev.header.timeMs = nowMs;
   }
}

void ReplicationSystem::DispatchImmediateEvent(const RepEvent& ev, const RepFrame& frame) const
{
   switch (ev.header.type)
   {
   case RepEventType::Spawn:
      FlushEvent_Spawn(ev, frame);
      break;
      
   case RepEventType::Despawn:
      FlushEvent_Despawn(ev, frame);
      break;
      
   case RepEventType::HealthChange:
      FlushEvent_Health(ev, frame);
      break;
      
   case RepEventType::MoneyChange:
      FlushEvent_Money(ev, frame);
      break;
      
      // TODO: 추가하고 여기서 연결하기 (작성도 해야 함, 멤버 함수)
   default:
      break;
   }
}

void ReplicationSystem::FlushEvent_Spawn(const RepEvent& ev, const RepFrame& frame) const
{
   const SpawnEvent* spawnEv = std::get_if<SpawnEvent>(&ev.payload);
   if (!spawnEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] Spawn event with invalid payload, skipping. objectId={}", ev.header.source.value);
      return;   // 페이로드가 SpawnEvent가 아닌 경우 (잘못된 이벤트)
   }
   
   se::room::N_EntitySpawn spawnPkt;
   auto* spawnInfo = spawnPkt.mutable_info();
   spawnInfo->set_type(spawnEv->type);
   spawnInfo->set_template_id(spawnEv->templateId);
   auto* entityIdPtr = spawnInfo->mutable_entity_id();
   entityIdPtr->set_value(ev.header.source.value);
   
   switch (spawnEv->type)
   {
   case ObjectType::OBJ_ITEM:
      {
         auto* itemInfo = spawnInfo->mutable_item_info();
         auto* pos = itemInfo->mutable_position();
         pos->set_x(spawnEv->position.x);
         pos->set_y(spawnEv->position.y);
         pos->set_z(spawnEv->position.z);
         auto* vel = itemInfo->mutable_velocity();
         vel->set_x(spawnEv->velocity.x);
         vel->set_y(spawnEv->velocity.y);
         vel->set_z(spawnEv->velocity.z);
         itemInfo->set_amount(spawnEv->amount);
      }
      break;
      
   case ObjectType::OBJ_PROJECTILE:
      {
         auto* projInfo = spawnInfo->mutable_projectile_info();
         auto* pos = projInfo->mutable_position();
         pos->set_x(spawnEv->position.x);
         pos->set_y(spawnEv->position.y);
         pos->set_z(spawnEv->position.z);
         auto* vel = projInfo->mutable_velocity();
         vel->set_x(spawnEv->velocity.x);
         vel->set_y(spawnEv->velocity.y);
         vel->set_z(spawnEv->velocity.z);
      }
      break;
      
   default:
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] Spawn event with unsupported object type {}, skipping. objectId={}", spawnEv->type, ev.header.source.value);
      break;
   }
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(spawnPkt);
   ownerRoom_->BroadcastReplication(sendBuffer);
}

void ReplicationSystem::FlushEvent_Despawn(const RepEvent& ev, const RepFrame& frame) const
{
   // 이건 아마 안쓰이지 않을까 싶기도 함...
   se::room::N_EntityDespawn despawnPkt;
   auto* entityIdPtr = despawnPkt.mutable_entity_id();
   entityIdPtr->set_value(ev.header.source.value);
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(despawnPkt);
   ownerRoom_->BroadcastReplication(sendBuffer);
}

void ReplicationSystem::FlushEvent_Health(const RepEvent& ev, const RepFrame& frame) const
{
   const HealthChangeEvent* healthChangeEv = std::get_if<HealthChangeEvent>(&ev.payload);
   if (!healthChangeEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] HealthChange event with invalid payload, skipping. PlayerId={}", ev.header.playerId);
      return;   // 페이로드가 HealthChangeEvent가 아닌 경우 (잘못된 이벤트)
   }
   
   se::game::N_HealthChanged healthPkt;
   auto* entityIdPtr = healthPkt.mutable_entity_id();
   entityIdPtr->set_value(ev.header.source.value);
   healthPkt.set_new_health(healthChangeEv->newHealth);
   healthPkt.set_delta(healthChangeEv->deltaHealth);
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(healthPkt);
   ownerRoom_->SendReplication(ev.header.playerId, sendBuffer);
}

void ReplicationSystem::FlushEvent_MaxHealth(const RepEvent& ev, const RepFrame& frame) const
{
   const MaxHealthChangeEvent* maxHealthChangeEv = std::get_if<MaxHealthChangeEvent>(&ev.payload);
   if (!maxHealthChangeEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] MaxHealthChange event with invalid payload, skipping. PlayerId={}", ev.header.playerId);
      return;   // 페이로드가 HealthChangeEvent가 아닌 경우 (잘못된 이벤트)
   }
   
   se::game::N_MaxHealthChanged maxHealthPkt;
   auto* entityIdPtr = maxHealthPkt.mutable_entity_id();
   entityIdPtr->set_value(ev.header.source.value);
   maxHealthPkt.set_new_max_health(maxHealthChangeEv->newMaxHealth);
   maxHealthPkt.set_new_current_health(maxHealthChangeEv->newHealth);
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(maxHealthPkt);
   ownerRoom_->SendReplication(ev.header.playerId, sendBuffer);
}

void ReplicationSystem::FlushEvent_Money(const RepEvent& ev, const RepFrame& frame) const
{
   const MoneyChangeEvent* moneyChangeEv = std::get_if<MoneyChangeEvent>(&ev.payload);
   if (!moneyChangeEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] MoneyChange event with invalid payload, skipping. PlayerId={}", ev.header.playerId);
      return;   // 페이로드가 MoneyChangeEvent가 아닌 경우 (잘못된 이벤트)
   }
   
   se::game::N_TimePointChanged timePointPkt;
   timePointPkt.set_new_time_points(moneyChangeEv->newMoney);
   timePointPkt.set_delta(moneyChangeEv->deltaMoney);
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(timePointPkt);
   ownerRoom_->SendReplication(ev.header.playerId, sendBuffer);
}
