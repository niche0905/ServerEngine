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
      
   case RepEventType::MaxHealthChange:
      FlushEvent_MaxHealth(ev, frame);
      break;
      
   case RepEventType::MoneyChange:
      FlushEvent_Money(ev, frame);
      break;
      
   case RepEventType::Explosion:
      FlushEvent_Explosion(ev, frame);
      break;
      
   case RepEventType::PickupItem:
      FlushEvent_PickupItem(ev, frame);
      break;
      
   case RepEventType::ItemChange:
      FlushEvent_Item(ev, frame);
      break;
      
   case RepEventType::Fire:
      FlushEvent_Fire(ev, frame);
      break;
      
   case RepEventType::Hit:
      FlushEvent_Hit(ev, frame);
      break;
      
   case RepEventType::WeaponStatChange:
      FlushEvent_WeaponStatChange(ev, frame);
      break;
      
   case RepEventType::ZoneFlow:
      FlushEvent_ZoneFlow(ev, frame);
      break;
      
      // TODO: 추가하고 여기서 연결하기 (작성도 해야 함, 멤버 함수)
   default:
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] Unsupported event type %u\n", static_cast<uint32>(ev.header.type));
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
      
// Testing
   case ObjectType::OBJ_CHEST:
      {
         auto* chestInfo = spawnInfo->mutable_chest_info();
         auto* pos = chestInfo->mutable_position();
         pos->set_x(spawnEv->position.x);
         pos->set_y(spawnEv->position.y);
         pos->set_z(spawnEv->position.z);
         chestInfo->set_yaw(spawnEv->yaw);
      }
      break;
      
   case ObjectType::OBJ_STORE:
      {
         auto* storeInfo = spawnInfo->mutable_store_info();
         auto* pos = storeInfo->mutable_position();
         pos->set_x(spawnEv->position.x);
         pos->set_y(spawnEv->position.y);
         pos->set_z(spawnEv->position.z);
         storeInfo->set_yaw(spawnEv->yaw);
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
      return;   // 페이로드가 MaxHealthChangeEvent가 아닌 경우 (잘못된 이벤트)
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

void ReplicationSystem::FlushEvent_Explosion(const RepEvent& ev, const RepFrame& frame) const
{
   const ExplosionEvent* explosionEv = std::get_if<ExplosionEvent>(&ev.payload);
   if (!explosionEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] Explosion event with invalid payload, skipping. objectId={}", ev.header.source.value);
      return;   // 페이로드가 ExplosionEvent가 아닌 경우 (잘못된 이벤트)
   }
   
   se::game::N_ProjectileExplosion explosionPkt;
   auto* entityIdPtr = explosionPkt.mutable_entity_id();
   entityIdPtr->set_value(ev.header.source.value);
   auto* posPtr = explosionPkt.mutable_position();
   posPtr->set_x(explosionEv->explosionPos.x);
   posPtr->set_y(explosionEv->explosionPos.y);
   posPtr->set_z(explosionEv->explosionPos.z);
   explosionPkt.set_explosion_radius(explosionEv->explosionRadius);
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(explosionPkt);
   ownerRoom_->BroadcastReplication(sendBuffer);
}

void ReplicationSystem::FlushEvent_PickupItem(const RepEvent& ev, const RepFrame& frame) const
{
   const ObjectId playerId = ev.header.source;
   const ObjectId itemObjectId = ev.header.target;
   
   se::game::N_PickupItem pickupItemPkt;
   auto* entityPtr = pickupItemPkt.mutable_entity_id();
   entityPtr->set_value(playerId.value);
   auto* itemIdPtr = pickupItemPkt.mutable_item_entity_id();
   itemIdPtr->set_value(itemObjectId.value);
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(pickupItemPkt);
   ownerRoom_->BroadcastReplication(sendBuffer);
}

void ReplicationSystem::FlushEvent_Item(const RepEvent& ev, const RepFrame& frame) const
{
   const ItemChangeEvent* itemChangeEv = std::get_if<ItemChangeEvent>(&ev.payload);
   if (!itemChangeEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] ItemChange event with invalid payload, skipping. PlayerId={}", ev.header.playerId);
      return;   // 페이로드가 ItemChangeEvent가 아닌 경우 (잘못된 이벤트)
   }
   
   SendBufferRef sendBuffer;
   
   if (itemChangeEv->deltaCount > 0) {
      se::game::N_ItemGained itemGainedPkt;
      itemGainedPkt.set_item_id(itemChangeEv->itemId);
      itemGainedPkt.set_new_quantity(itemChangeEv->newCount);
      itemGainedPkt.set_quantity(itemChangeEv->deltaCount);
      
      sendBuffer = ServerPacketHandler::MakeSendBuffer(itemGainedPkt);
   }
   else {
      se::game::N_ItemLost itemLostPkt;
      itemLostPkt.set_item_id(itemChangeEv->itemId);
      itemLostPkt.set_new_quantity(itemChangeEv->newCount);
      itemLostPkt.set_quantity(itemChangeEv->deltaCount);
      
      sendBuffer = ServerPacketHandler::MakeSendBuffer(itemLostPkt);
   }
   
   ownerRoom_->SendReplication(ev.header.playerId, sendBuffer);
}

void ReplicationSystem::FlushEvent_Fire(const RepEvent& ev, const RepFrame& frame) const
{
   const FireEvent* fireEv = std::get_if<FireEvent>(&ev.payload);
   if (!fireEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] Fire event with invalid payload, skipping. PlayerId={}", ev.header.playerId);
      return;   // 페이로드가 FireEvent가 아닌 경우 (잘못된 이벤트)
   }
   
   se::game::N_Fire noti;
   auto* entityIdPtr = noti.mutable_entity_id();
   entityIdPtr->set_value(ev.header.source.value);
      
   noti.set_weapon_id(fireEv->weaponId);
   noti.set_shot_seed(fireEv->shotSeed);
      
   auto* startPosPtr = noti.mutable_start_position();
   startPosPtr->set_x(fireEv->startPos.x);
   startPosPtr->set_y(fireEv->startPos.y);
   startPosPtr->set_z(fireEv->startPos.z);
      
   auto* dirPtr = noti.mutable_direction();
   dirPtr->set_x(fireEv->direction.x);
   dirPtr->set_y(fireEv->direction.y);
   dirPtr->set_z(fireEv->direction.z);
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   ownerRoom_->BroadcastReplication(sendBuffer, ev.header.playerId);
}

void ReplicationSystem::FlushEvent_Hit(const RepEvent& ev, const RepFrame& frame) const
{
   const HitEvent* hitEv = std::get_if<HitEvent>(&ev.payload);
   if (!hitEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] Hit event with invalid payload, skipping. objectId={}", ev.header.source.value);
      return;   // 페이로드가 HitEvent가 아닌 경우 (잘못된 이벤트)
   }
   
   se::game::N_EntityHit noti;
   auto* entityIdPtr = noti.mutable_entity_id();
   entityIdPtr->set_value(ev.header.source.value);
   auto* hitPosPtr = noti.mutable_hit_position();
   hitPosPtr->set_x(hitEv->hitPos.x);
   hitPosPtr->set_y(hitEv->hitPos.y);
   hitPosPtr->set_z(hitEv->hitPos.z);
   noti.set_damage(hitEv->damage);
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   ownerRoom_->BroadcastReplication(sendBuffer);
}

void ReplicationSystem::FlushEvent_WeaponStatChange(const RepEvent& ev, const RepFrame& frame) const
{
   const WeaponStatChangeEvent* weaponStatChangeEv = std::get_if<WeaponStatChangeEvent>(&ev.payload);
   if (!weaponStatChangeEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] WeaponStatChange event with invalid payload, skipping. PlayerId={}", ev.header.playerId);
      return;   // 페이로드가 WeaponStatChangeEvent가 아닌 경우 (잘못된 이벤트)
   }
   
   se::game::N_WeaponStatChanged noti;
   noti.set_weapon_id(weaponStatChangeEv->weaponId);
   
   if (weaponStatChangeEv->modifier.magCapacityDelta != 0) {
      se::game::WeaponStatValue* magCapacityVal = noti.add_stats();
      magCapacityVal->set_stat_type(se::game::WeaponStatType::WEAPON_STAT_MAGAZINE_SIZE);
      magCapacityVal->set_int_value(weaponStatChangeEv->modifier.magCapacityDelta);
   }
   
   if (not SE::Math::NearlyZero(weaponStatChangeEv->modifier.fireIntervalSecDelta)) {
      se::game::WeaponStatValue* fireRateVal = noti.add_stats();
      fireRateVal->set_stat_type(se::game::WeaponStatType::WEAPON_STAT_FIRE_INTERVAL);
      fireRateVal->set_float_value(weaponStatChangeEv->modifier.fireIntervalSecDelta);
   }
   
   if (not SE::Math::NearlyZero(weaponStatChangeEv->modifier.reloadTimeSecDelta)) {
      se::game::WeaponStatValue* reloadTimeVal = noti.add_stats();
      reloadTimeVal->set_stat_type(se::game::WeaponStatType::WEAPON_STAT_RELOAD);
      reloadTimeVal->set_float_value(weaponStatChangeEv->modifier.reloadTimeSecDelta);
   }
   
   if (weaponStatChangeEv->modifier.palletCountDelta != 0) {
      se::game::WeaponStatValue* palletCountVal = noti.add_stats();
      palletCountVal->set_stat_type(se::game::WeaponStatType::WEAPON_STAT_PALLET);
      palletCountVal->set_int_value(weaponStatChangeEv->modifier.palletCountDelta);
   }
   
   if (not SE::Math::NearlyZero(weaponStatChangeEv->modifier.coneAngleDegreesDelta)) {
      se::game::WeaponStatValue* coneAngleVal = noti.add_stats();
      coneAngleVal->set_stat_type(se::game::WeaponStatType::WEAPON_STAT_CONE);
      coneAngleVal->set_float_value(weaponStatChangeEv->modifier.coneAngleDegreesDelta);
   }
   
   if (not SE::Math::NearlyZero(weaponStatChangeEv->modifier.projectileSpeedDelta)) {
      se::game::WeaponStatValue* projSpeedVal = noti.add_stats();
      projSpeedVal->set_stat_type(se::game::WeaponStatType::WEAPON_STAT_PROJECTILE_SPEED);
      projSpeedVal->set_float_value(weaponStatChangeEv->modifier.projectileSpeedDelta);
   }
   
   if (not SE::Math::NearlyZero(weaponStatChangeEv->modifier.explosionRadiusDelta)) {
      se::game::WeaponStatValue* explosionRadiusVal = noti.add_stats();
      explosionRadiusVal->set_stat_type(se::game::WeaponStatType::WEAPON_STAT_EXPLOSION_RADIUS);
      explosionRadiusVal->set_float_value(weaponStatChangeEv->modifier.explosionRadiusDelta);
   }
   
   SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
   ownerRoom_->SendReplication(ev.header.playerId, sendBuffer);
}

void ReplicationSystem::FlushEvent_ZoneFlow(const RepEvent& ev, const RepFrame& frame) const
{
   const ZoneFlowEvent* zoneFlowEv = std::get_if<ZoneFlowEvent>(&ev.payload);
   if (!zoneFlowEv) {
      consoleLogger->Log(Color::Yellow, L"[ReplicationSystem] ZoneFlow event with invalid payload, skipping.\n");
      return;   // 페이로드가 ZoneFlowEvent가 아닌 경우 (잘못된 이벤트)
   }

   SendBufferRef sendBuffer;
   if (zoneFlowEv->flowing) {
      se::test::N_ZoneStart zoneStartPkt;
      sendBuffer = ServerPacketHandler::MakeSendBuffer(zoneStartPkt);
   }
   else {
      se::test::N_ZoneStop zoneStopPkt;
      sendBuffer = ServerPacketHandler::MakeSendBuffer(zoneStopPkt);
   }
   
   ownerRoom_->BroadcastReplication(sendBuffer);
}
