#include "pch.h"
#include "PlayerPawn.h"
#include "Content/Gameplay/Collider/ColliderComponent.h"
#include "Content/Gameplay/Combat/PlayerCombatComponent.h"
#include "Data/GameDataManager.h"
#include "Physics/Collider/CharacterCapsuleCollider.h"
#include "Service/Room/Room.h"

namespace 
{
   WeaponStatModifier MakeWeaponStatFinal(uint32 weaponId, WeaponStat oldStat, WeaponStat newStat)
   {
      WeaponStatModifier finalStat{};
      
      if (oldStat.common.magCapacity != newStat.common.magCapacity) {
         finalStat.magCapacityDelta = newStat.common.magCapacity;
      }
      
      if (!SE::Math::NearlyEqual(oldStat.common.fireIntervalSec, newStat.common.fireIntervalSec)) {
         finalStat.fireIntervalSecDelta = newStat.common.fireIntervalSec;
      }
      
      if (!SE::Math::NearlyEqual(oldStat.common.reloadTimeSec, newStat.common.reloadTimeSec)) {
         finalStat.reloadTimeSecDelta = newStat.common.reloadTimeSec;
      }
      
      if (weaponId == 2) {
         auto* oldShotgunStat =  std::get_if<ShotgunStat>(&oldStat.extra);
         auto* newShotgunStat =  std::get_if<ShotgunStat>(&newStat.extra);
         
         if (oldShotgunStat and newShotgunStat) {
            if (oldShotgunStat->pelletCount != newShotgunStat->pelletCount) {
               finalStat.palletCountDelta = newShotgunStat->pelletCount;
            }
            
            if (!SE::Math::NearlyEqual(oldShotgunStat->coneAngleDegrees, newShotgunStat->coneAngleDegrees)) {
               finalStat.coneAngleDegreesDelta = newShotgunStat->coneAngleDegrees;
            }
         }
      }
      if (weaponId == 3)
      {
         auto* oldLauncherStat =  std::get_if<LauncherStat>(&oldStat.extra);
         auto* newLauncherStat =  std::get_if<LauncherStat>(&newStat.extra);
         
         if (oldLauncherStat and newLauncherStat) {
            if (oldLauncherStat->projectileSpeed != newLauncherStat->projectileSpeed) {
               finalStat.projectileSpeedDelta = newLauncherStat->projectileSpeed;
            }
            
            if (!SE::Math::NearlyEqual(oldLauncherStat->explosionRadius, newLauncherStat->explosionRadius)) {
               finalStat.explosionRadiusDelta = newLauncherStat->explosionRadius;
            }
         }
      }
      
      return finalStat;
   }
}

/*--------------
   PlayerPawn
--------------*/

int32 PlayerPawn::ResolveIncomingDamage(int32 amount, const DamageContext& ctx)
{
   constexpr int32 kMultiplierForZoneDamage = 10;   // TEMP: 존 데미지에 대한 데미지 배율 (예: 1 데미지당 10 화폐 삭제)
   
   if (ctx.type == DamageType::Zone) {
      auto room = GetRoom();
      if (!room)
         return amount;   // 방이 없는 경우에는 존 데미지 처리하지 않음
      
      const int32 deleteMoney = amount * kMultiplierForZoneDamage;
      
      
      if (CanSpend(CurrencyType::TimePoint, deleteMoney)) {
         SpendMoney(CurrencyType::TimePoint, deleteMoney, MoneyChangeContext{
            .reason = MoneyChangeReason::ZoneDamage,
         });
         return 0;   // 존 데미지로 삭제할 재화가 충분하면 체력에는 데미지를 주지 않음
      }
      
      // TODO: 존 데미지는 체력이 아니라 재화를 달게 한다 
      //       만약 재화의 차감으로도 죽을 수 있다면 사망 처리 진행
      //       재화가 모두 삭제되었더라도 죽이지 않을 것이라면 차감된 재화 양에 따라 amount 수정
      
      int32 currentMoney = GetBalance(CurrencyType::TimePoint);
      SpendMoney(CurrencyType::TimePoint, currentMoney, MoneyChangeContext{
           .reason = MoneyChangeReason::ZoneDamage,
        });
      int64 remainDelete = deleteMoney - currentMoney;
      int32 remainAmount = static_cast<int32>((remainDelete + kMultiplierForZoneDamage - 1) / kMultiplierForZoneDamage);
      
      return remainAmount;   // 존 데미지로 삭제할 재화가 부족해서 체력에도 데미지를 줘야 하는 경우 남은 데미지 양 반환
   }
   
   return Pawn::ResolveIncomingDamage(amount, ctx);
}

void PlayerPawn::Damaged(const DamageResult& dmgResult)
{
   Pawn::Damaged(dmgResult);
   if (auto room = GetRoom()) {
      room->NotifyHealthChange(GetOwnerPlayerId(), dmgResult.hpAfter, -dmgResult.applied);
   }
}

void PlayerPawn::Heal(int32 amount)
{
   int32 currentHp = GetHealth().GetHp();
   health_.Heal(amount);
   int32 newHp = GetHealth().GetHp();
   if (auto room = GetRoom()) {
      room->NotifyHealthChange(GetOwnerPlayerId(), newHp, newHp - currentHp);
   }
}

void PlayerPawn::OnPreRespawn(ObjectManager& om)
{
   Pawn::OnPreRespawn(om);
   
   // TODO: Inventory Save 꺼 가져오기
   //       Inventory 외에도 여러 항목들 가져와서 세팅하기 (예: 스킬, 업그레이드, 위치 등)
}

void PlayerPawn::OnPostRespawn(ObjectManager& om)
{
   Pawn::OnPostRespawn(om);
   
   MarkReplicationDirty(ReplicationDirty::Inventory);    // 인벤토리 새로고침
}

LootBundle PlayerPawn::GenerateDrops()
// 플레이어의 경우는 죽었을 때 인벤토리에 있는 아이템을 전부 드롭한다
{
   if (inventory_.GetUsedSlots() == 0)
      return LootBundle{};   // 드롭할 아이템이 없는 경우 빈 LootBundle 반환
   
   auto slots = inventory_.GetSlots();
   LootBundle bundle;
   
   for (const auto& slot : slots) {
      if (slot.IsValid()) {
         bundle.AddItem(slot.id, slot.count);
      }
   }
   
   return bundle;
}

InventoryOpResult PlayerPawn::AddItem(ItemId itemId, int32 count, const ItemChangeContext& ctx)
{
   InventoryOpResult result = inventory_.AddItem(itemId, count, ctx);
   if (auto room = GetRoom()) {
      room->NotifyItemChange(GetOwnerPlayerId(), itemId, result.newQuantity, result.delta.count);
   }
   return result;
}

InventoryOpResult PlayerPawn::RemoveItem(ItemId itemId, int32 count, const ItemChangeContext& ctx)
{
   InventoryOpResult result = inventory_.RemoveItem(itemId, count, ctx);
   if (auto room = GetRoom()) {
      room->NotifyItemChange(GetOwnerPlayerId(), itemId, result.newQuantity, result.delta.count);
   }
   return result;
}

InventoryOpResult PlayerPawn::ConsumeItem(ItemId itemId, int32 count, const ItemChangeContext& ctx)
{
   InventoryOpResult result = inventory_.ConsumeItem(itemId, count, ctx);
   if (auto room = GetRoom()) {
      room->NotifyItemChange(GetOwnerPlayerId(), itemId, result.newQuantity, result.delta.count);
   }
   return result;
}

MoneyChangeResult PlayerPawn::AddMoney(CurrencyType currency, CurrencyAmount amount,
   const MoneyChangeContext& ctx)
{
   MoneyChangeResult result = wallet_.AddMoney(currency, amount, ctx);
   if (auto room = GetRoom()) {
      room->NotifyTimePointChange(GetOwnerPlayerId(), result.after, result.delta);
   }
   return result;
}

MoneyChangeResult PlayerPawn::SpendMoney(CurrencyType currency, CurrencyAmount amount,
   const MoneyChangeContext& ctx)
{
   MoneyChangeResult result = wallet_.SpendMoney(currency, amount, ctx);
   if (auto room = GetRoom()) {
      room->NotifyTimePointChange(GetOwnerPlayerId(), result.after, result.delta);
   }
   return result;
}

void PlayerPawn::OnSkillChanged(SkillId skillId)
{
   MarkReplicationDirty(ReplicationDirty::SkillState);
   // TODO: 작성하기
}

void PlayerPawn::OnWeaponUpgradeApplied(WeaponUpgradeCode code)
{
   auto* playerCombat = GetPlayerCombat();
   if (!playerCombat)
      return;
   
   RefreshWeaponStatsByUpgrade(code);
   
   playerCombat->OnWeaponUpgrade();
}

void PlayerPawn::RefreshWeaponStatsByUpgrade(uint32 code)
{
   auto room = GetRoom();
   auto* playerCombat = GetPlayerCombat();
   auto* GDM = room->GetGameDataManager();
   
   if (!room or !playerCombat or !GDM)
      return;
   
   const auto& upgradeDef = GDM->GetUpgradeTable().WeaponUpgradeTable.Find(code);
   uint32 weaponId = upgradeDef->target.weaponId;
   
   if (auto* weaponSlot = playerCombat->GetWeaponSlot(weaponId))
      weaponSlot->dirty = true;
   
   auto& weaponSystem = room->GetRoomGameSystem().GetWeaponSystem();
   
   for (uint32 weaponId = 1; weaponId <= MaxWeaponSlots; ++weaponId) {
      auto* weaponSlot = playerCombat->GetWeaponSlot(weaponId);
      if (!weaponSlot)
         continue;
      
      if (not weaponSlot->dirty)
         continue;
      
      WeaponStat oldStat = weaponSlot->stat;
      WeaponStat finalStat;
      if (!weaponSystem.BuildFinalWeaponStat(this, weaponId, finalStat))
         continue;
      
      WeaponStatModifier finalWeaponStat = MakeWeaponStatFinal(weaponId, oldStat, finalStat);
      
      weaponSlot->stat = finalStat;
      room->NotifyWeaponStatChange(GetOwnerPlayerId(), weaponId, finalWeaponStat);
   }
}

void PlayerPawn::OnStatUpgradeApplied(StatUpgradeCode code, int32 newLevel)
{
   auto room = GetRoom();
   if (!room)
      return;
   
   auto& upgradeSystem = room->GetRoomGameSystem().GetUpgradeSystem();
   int32 finalValue = upgradeSystem.GetStatFinalValue(code, newLevel);
   
   switch (code)
   {
   case Health_S:
      health_.UpgradeHealth(finalValue);
      break;
   case Speed_S:
      speed_ = finalValue;
      MarkReplicationDirty(ReplicationDirty::Stat);
      break;
   }
}

bool PlayerPawn::TrySetSavePoint(const Vector3& location)
{
   // TODO: 쿨타임 체크 진행하기
   //       쿨타임 통과 할 시
   
   // TODO: Player의 위치와 세이브 포인트 위치의 차가 너무 크지 않은지 체크하기
   
   // TODO: 현재 정보에서 저장해야 할 것들 저장하기
   //       플레이어 상태 (체력, 인벤토리, 스킬, 업데이트 등)
   //       Save Component를 작성하는 것이 좋아 보인다
   {
      SetSavedRespawnPosition(location);
   }
   
   return true;
}

void PlayerPawn::OnSpawn()
{
   Pawn::OnSpawn();
   
   combat_ = std::make_unique<PlayerCombatComponent>();
   if (combat_)
      combat_->Init(this);
   
   // TEMP
   {
      auto bodyCollider = std::make_unique<ColliderComponent>();
      auto capsuleCollider = std::make_unique<SE::Physics::CharacterCapsuleCollider>(SE::Math::Vector3{0.0f, 0.0f, -90.0f}, 180.0f, 35.0f);
      bodyCollider->Init(this, ColliderRole::Hurtbox, std::move(capsuleCollider));
      
      colliders_.push_back(std::move(bodyCollider));
   }
   
   respawn_.Init(this, RespawnPolicy{});
   inventory_.Init(this, 20);
   wallet_.Init(this);     
   wallet_.SetBalanceUnsafe(CurrencyType::TimePoint, 1000);      // TEMP: 초기 재화 1000
                                                                        // TODO: 이 값 Config로 빼기
   skill_.Init(this);
   upgrade_.Init(this);
   
   InitDefaultLoadout();
}

void PlayerPawn::OnPreDestroy()
{
   Pawn::OnPreDestroy();
}

void PlayerPawn::Tick(float dt)
{
   Pawn::Tick(dt);
   
   (void)dt;
}

void PlayerPawn::OnDeath(ObjectManager& om, const DamageResult& dmgResult)
{
   Pawn::OnDeath(om, dmgResult);
   
   StartDeadState(om, dmgResult);
}

void PlayerPawn::StartDeadState(ObjectManager& om, const DamageResult& dmgResult)
{
   SetVelocity(Vector3{});
   
   // TODO: 드롭된 아이템과 화폐를 월드에 스폰하는 로직 추가
   // (om, drops);
}

void PlayerPawn::InitDefaultLoadout()
{
   auto room = GetRoom();
   auto* playerCombat = GetPlayerCombat();
   if (!room or !playerCombat)
      return;
   
   auto& weaponSystem = room->GetRoomGameSystem().GetWeaponSystem();
   
   weaponSystem.CreateInitialWeaponSlot(this, 1, *playerCombat->GetWeaponSlot(1));
   weaponSystem.CreateInitialWeaponSlot(this, 2, *playerCombat->GetWeaponSlot(2));
   weaponSystem.CreateInitialWeaponSlot(this, 3, *playerCombat->GetWeaponSlot(3));
}
