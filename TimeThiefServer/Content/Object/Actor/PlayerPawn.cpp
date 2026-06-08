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
   if (ctx.type == DamageType::Zone) {
      auto room = GetRoom();
      if (!room)
         return amount;   // 방이 없는 경우에는 존 데미지 처리하지 않음
      
      const int32 deleteMoney = amount * zoneDamageTimePointMultiplier_;
      
      
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
      int32 remainAmount = static_cast<int32>((remainDelete + zoneDamageTimePointMultiplier_ - 1) / zoneDamageTimePointMultiplier_);
      
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

void PlayerPawn::SetSpeed(int32 speed)
{
   speed_ = std::max(0, speed);
   MarkReplicationDirty(ReplicationDirty::Stat);
}

void PlayerPawn::OnPreRespawn(ObjectManager& om)
{
   Pawn::OnPreRespawn(om);
   
   save_.Rollback();
}

void PlayerPawn::OnPostRespawn(ObjectManager& om)
{
   Pawn::OnPostRespawn(om);
   ResetTimeRewindHistory();
}

bool PlayerPawn::TryReserveRespawn()
{
   MoneyChangeContext ctx{ MoneyChangeReason::RespawnCost };
   MoneyChangeResult result = SpendMoney(CurrencyType::TimePoint, respawnCostTimePoint_, ctx);
   
   return result.accepted;    // 리스폰 비용을 지불하였으면
                              // 일단은 Respawn 예약을 할 때 비용을 지불하는 것으로
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
   if (result.delta == 0)
      return result;   // 변화량이 없는 경우에는 방에 알리지 않음
   
   if (auto room = GetRoom()) {
      room->NotifyTimePointChange(GetOwnerPlayerId(), result.after, result.delta);
   }
   return result;
}

MoneyChangeResult PlayerPawn::SpendMoney(CurrencyType currency, CurrencyAmount amount,
   const MoneyChangeContext& ctx)
{
   MoneyChangeResult result = wallet_.SpendMoney(currency, amount, ctx);
   if (result.delta == 0)
      return result;   // 변화량이 없는 경우에는 방에 알리지 않음
   
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
   
   auto& weaponSystem = room->GetRoomGameSystem().GetWeaponSystem();
   
   auto* weaponSlot = playerCombat->GetWeaponSlot(weaponId);
   if (!weaponSlot)
      return;
      
   WeaponStat oldStat = weaponSlot->stat;
   WeaponStat finalStat;
   if (!weaponSystem.BuildFinalWeaponStat(this, weaponId, finalStat))
      return;
      
   WeaponStatModifier finalWeaponStat = MakeWeaponStatFinal(weaponId, oldStat, finalStat);
      
   weaponSlot->stat = finalStat;
   room->NotifyWeaponStatChange(GetOwnerPlayerId(), weaponId, finalWeaponStat);
}

void PlayerPawn::RefreshWeaponStats()
{
   MarkReplicationDirty(ReplicationDirty::WeaponStat);
   
   auto room = GetRoom();
   auto* playerCombat = GetPlayerCombat();
   auto& weaponSystem = room->GetRoomGameSystem().GetWeaponSystem();
   
   if (!room or !playerCombat)
      return;
   
   for (uint32 weaponId = 1; weaponId <= MaxWeaponSlots; ++weaponId) {
      auto* weaponSlot = playerCombat->GetWeaponSlot(weaponId);
      if (!weaponSlot)
         continue;
      
      WeaponStat finalStat;
      if (!weaponSystem.BuildFinalWeaponStat(this, weaponId, finalStat))
         continue;
      
      weaponSlot->stat = finalStat;
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
   save_.CaptureSnapshot();
   SetSavedRespawnPosition(location);
   
   return true;
}

void PlayerPawn::ApplyTimeAccel(uint32 moveSpeedBonusPercent, uint32 combatSpeedBonusPercent)
{
   RestoreTimeAccelSnapshot();

   ++timeAccelToken_;
   timeAccelActive_ = true;
   timeAccelMoveSpeedMultiplier_ = 1.0f + static_cast<float>(moveSpeedBonusPercent) / 100.0f;

   if (auto* playerCombat = GetPlayerCombat()) {
      const float combatSpeedMultiplier = 1.0f + static_cast<float>(combatSpeedBonusPercent) / 100.0f;
      playerCombat->SetTimeAccelCombatSpeedMultiplier(combatSpeedMultiplier);
   }
}

void PlayerPawn::ClearTimeAccel(uint64 token)
{
   if (!timeAccelActive_)
      return;

   if (token != 0 and token != timeAccelToken_)
      return;

   RestoreTimeAccelSnapshot();
}

void PlayerPawn::RestoreTimeAccelSnapshot()
{
   if (!timeAccelActive_)
      return;

   timeAccelMoveSpeedMultiplier_ = 1.0f;
   if (auto* playerCombat = GetPlayerCombat()) {
      playerCombat->ClearTimeAccelCombatSpeedMultiplier();
   }

   timeAccelActive_ = false;
}

PlayerPawn::TimeRewindFrame PlayerPawn::MakeCurrentTimeRewindFrame() const
{
   return TimeRewindFrame{
      .hp = GetHealth().GetHp(),
      .position = GetPosition(),
   };
}

void PlayerPawn::ResetTimeRewindHistory()
{
   const TimeRewindFrame frame = MakeCurrentTimeRewindFrame();
   timeRewindHistory_.fill(frame);
   timeRewindNextIndex_ = 0;
   timeRewindValidCount_ = TimeRewindHistoryCapacity;
   timeRewindSampleAccumSec_ = 0.0f;
}

void PlayerPawn::PushTimeRewindFrame()
{
   timeRewindHistory_[timeRewindNextIndex_] = MakeCurrentTimeRewindFrame();
   timeRewindNextIndex_ = (timeRewindNextIndex_ + 1) % TimeRewindHistoryCapacity;
   timeRewindValidCount_ = std::min(timeRewindValidCount_ + 1, TimeRewindHistoryCapacity);
}

void PlayerPawn::TickTimeRewindHistory(float dt)
{
   if (dt <= 0.0f)
      return;

   timeRewindSampleAccumSec_ += dt;
   const float sampleIntervalSec = static_cast<float>(TimeRewindSampleIntervalMs) / 1000.0f;

   while (timeRewindSampleAccumSec_ >= sampleIntervalSec) {
      PushTimeRewindFrame();
      timeRewindSampleAccumSec_ -= sampleIntervalSec;
   }
}

bool PlayerPawn::TryGetTimeRewindFrame(uint32 rewindDurationMs, TimeRewindFrame& outFrame) const
{
   if (timeRewindValidCount_ == 0)
      return false;

   const uint32 samplesBack = std::max<uint32>(1, (rewindDurationMs + TimeRewindSampleIntervalMs - 1) / TimeRewindSampleIntervalMs);
   const size_t clampedSamplesBack = std::min<size_t>(samplesBack, timeRewindValidCount_);
   const size_t newestIndex = (timeRewindNextIndex_ + TimeRewindHistoryCapacity - 1) % TimeRewindHistoryCapacity;
   const size_t offset = clampedSamplesBack - 1;
   const size_t targetIndex = (newestIndex + TimeRewindHistoryCapacity - (offset % TimeRewindHistoryCapacity)) % TimeRewindHistoryCapacity;

   outFrame = timeRewindHistory_[targetIndex];
   return true;
}

bool PlayerPawn::RestoreTimeRewind(uint32 rewindDurationMs, TimeRewindFrame* outFrame)
{
   TimeRewindFrame frame{};
   if (!TryGetTimeRewindFrame(rewindDurationMs, frame))
      return false;

   GetHealth().SetHpUnsafe(frame.hp);
   // SetPosition(frame.position);

   if (outFrame)
      *outFrame = frame;

   ResetTimeRewindHistory();
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
   wallet_.SetBalanceUnsafe(CurrencyType::TimePoint, initialTimePoint_);
   skill_.Init(this);
   upgrade_.Init(this);
   
   save_.Init(this);
   save_.CaptureSnapshot();   // 초기 상태 저장
   ResetTimeRewindHistory();
   
   deathCount_ = 0;
   
   InitDefaultLoadout();
}

void PlayerPawn::OnPreDestroy()
{
   Pawn::OnPreDestroy();
}

void PlayerPawn::Tick(float dt)
{
   // Pawn::Tick(dt);
   
   TickTimeRewindHistory(dt);
}

void PlayerPawn::OnDeath(ObjectManager& om, const DamageContext& ctx, const DamageResult& dmgResult)
{
   Pawn::OnDeath(om, ctx, dmgResult);
   
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
