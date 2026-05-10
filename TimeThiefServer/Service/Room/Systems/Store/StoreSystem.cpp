#include "pch.h"
#include "StoreSystem.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Service/Room/Room.h"

namespace 
{
   constexpr float kStoreInteractionDistance = 300.0f;   // TEMP: s임시 값
   
}

/*----------------
   StoreSystem
----------------*/

bool StoreSystem::Init(Room* ownerRoom, const StoreEntryTable& storeEntryTable)
{
   if (!ownerRoom)
      return false;   // 유효하지 않은 ownerRoom
   
   ownerRoom_ = ownerRoom;
   storeEntryTable_ = &storeEntryTable;
   return true;
}

StoreBuyResult StoreSystem::Buy(const StoreBuyRequest& req)
{
   StoreBuyResult result;
   result.playerId = req.playerId;
   result.storeId = req.storeId;
   result.entryId = req.entryId;
   
   if (!ownerRoom_)
      return result;
   
   StoreBuyContext ctx{};
   StoreBuyResult validateResult = ValidateBuyRequest(req, ctx);
   if (!validateResult.success)
      return validateResult;
   
   if (!TryConsumeCost(ctx)) {
      result.resultCode = StoreBuyResultCode::NotEnoughCurrency;
      return result;
   }
   
   if (!TryApplyReward(ctx)) {
      result.resultCode = StoreBuyResultCode::ApplyFailed;
      return result;
   }
   
   result.success = true;
   result.resultCode = StoreBuyResultCode::Success;
   return result;
}

StoreBuyResult StoreSystem::ValidateBuyRequest(const StoreBuyRequest& req, StoreBuyContext& ctx)
{
   StoreBuyResult result;
   result.playerId = req.playerId;
   result.storeId = req.storeId;
   result.entryId = req.entryId;
   
   if (!ownerRoom_)
      return result;
   
   auto& om = ownerRoom_->GetObjectManager();
   
   auto* player = om.FindAs<PlayerPawn>(req.playerId);
   if (!player) {
      result.resultCode = StoreBuyResultCode::InvalidPlayer;
      return result;
   }
   
   auto* store = om.FindAs<Actor>(req.storeId);
   if (!store) {
      result.resultCode = StoreBuyResultCode::InvalidStore;
      return result;
   }
   
   if (!store->IsStore()) {
      result.resultCode = StoreBuyResultCode::InvalidStore;
      return result;
   }
   
   if (!storeEntryTable_->IsValidEntry(result.entryId)) {
      result.resultCode = StoreBuyResultCode::InvalidEntry;
      return result;
   }
   
   const StoreEntryDef* entryDef = FindStoreEntry(req.entryId);
   if (entryDef == nullptr) {
      result.resultCode = StoreBuyResultCode::InvalidEntry;
      return result;
   }
   
   if (!CanInteractStore(player, store)) {
      result.resultCode = StoreBuyResultCode::TooFar;
      return result;
   }
   
   if (!CanBuy(player)) {
      result.resultCode = StoreBuyResultCode::InvalidState;
      return result;
   }
   
   if (!CanPurchaseReward(player, entryDef)) {
      result.resultCode = StoreBuyResultCode::PurchaseLimitExceeded;
      return result;
   }
   
   ctx.playerPawn = player;
   ctx.storeActor = store;
   ctx.entryId = req.entryId;
   ctx.entryDef = entryDef;
   
   result.success = true;
   result.resultCode = StoreBuyResultCode::Success;
   return result;
}

bool StoreSystem::TryConsumeCost(StoreBuyContext& ctx)
{
   if (!ctx.playerPawn)
      return false;
   
   MoneyChangeResult result = ctx.playerPawn->SpendMoney(CurrencyType::TimePoint, ctx.entryDef->cost, MoneyChangeContext{MoneyChangeReason::Purchase, ctx.entryId});
   
   return result.accepted;
}

bool StoreSystem::TryApplyReward(StoreBuyContext& ctx)
{
   if (!ctx.playerPawn or !ctx.entryDef)
      return false;
   
   // TODO: 아이템 적용 필요 (아이템은 Inventory 추가, 스킬은 소유, 무기 강화, 스텟 강화 처리 등)
   switch (ctx.entryDef->rewardType)
   {
   case StoreRewardType::Item:
      // 아이템 보상 처리 (인벤토리에 아이템 추가)
      return TryApplyItemReward(ctx);
      
   case StoreRewardType::Skill:
      // 스킬 보상 처리 (스킬 해금)
      return TryApplySkillReward(ctx);
      
   case StoreRewardType::WeaponUpgrade:
      // 무기 강화 보상 처리 (무기 강화)
      return TryApplyWeaponUpgradeReward(ctx);
      
   case StoreRewardType::StatUpgrade:
      // 스텟 강화 보상 처리 (스텟 강화)
      return TryApplyStatUpgradeReward(ctx);
      
   default:
      return false;
   }
}

const StoreEntryDef* StoreSystem::FindStoreEntry(uint32 entryId) const
{
   if (entryId == 0 or !storeEntryTable_)
      return nullptr;
   
   return storeEntryTable_->GetStoreEntry(entryId);
}

bool StoreSystem::CanInteractStore(PlayerPawn* playerPawn, Actor* storeActor) const
{
   if (!playerPawn || !storeActor)
      return false;
   
   const auto& playerPos = playerPawn->GetPosition();
   const auto& storePos = storeActor->GetPosition();
   const float distSq = (playerPos - storePos).LengthSq();
   
   return distSq <= (kStoreInteractionDistance * kStoreInteractionDistance);
}

bool StoreSystem::CanBuy(PlayerPawn* playerPawn) const
{
   if (!playerPawn)
      return false;
   
   if (not playerPawn->IsHpAlive())
      return false;
   
   // 다른 Player 상태 체크도 필요하다면 추가하기
   
   return true;
}

bool StoreSystem::CanPurchaseReward(PlayerPawn* playerPawn, const StoreEntryDef* entryDef) const
{
   if (!playerPawn or !entryDef)
      return false;
   
   switch (entryDef->rewardType)
   {
   case StoreRewardType::Item:
      return true;      // 아이템은 항상 구매 가능
      
   case StoreRewardType::Skill:
      {
         auto& skillComp = playerPawn->GetSkill();
         return skillComp.CanUnlockSkill(entryDef->skillId);
      }
      
   case StoreRewardType::WeaponUpgrade:
      {
         auto& upgradeComp = playerPawn->GetUpgrade();
         return upgradeComp.CanApplyWeaponUpgrade(entryDef->weaponUpgradeType);
      }
   case StoreRewardType::StatUpgrade:
      {
         auto& upgradeComp = playerPawn->GetUpgrade();
         if (entryDef->upgradeLineId == 0)
            return false;
   
         int32 maxLevel = storeEntryTable_->GetMaxLevel(entryDef->upgradeLineId);
         if (maxLevel <= 0)
            return false;
         return upgradeComp.CanApplyStatUpgrade(entryDef->statUpgradeType, maxLevel);
      }
   default:
      return false;
   }
}

bool StoreSystem::TryApplyItemReward(const StoreBuyContext& ctx)
{
   const StoreEntryDef* entryDef = ctx.entryDef;
   ItemChangeContext changeCtx{ ItemChangeReason::Purchase, ctx.entryId };
   InventoryOpResult result = ctx.playerPawn->AddItem(entryDef->itemId, entryDef->itemCount, changeCtx);
   return result.accepted;
}

bool StoreSystem::TryApplySkillReward(const StoreBuyContext& ctx)
{
   // TODO: 아래 Player Pawn으로 옮기고 Replication으로 클라이언트에 전달하기
   SkillComponent& skillComp = ctx.playerPawn->GetSkill();
   const StoreEntryDef* entryDef = ctx.entryDef;
   return skillComp.UnlockSkill(entryDef->skillId);
}

bool StoreSystem::TryApplyWeaponUpgradeReward(const StoreBuyContext& ctx)
{
   UpgradeComponent& upgradeComp = ctx.playerPawn->GetUpgrade();
   const StoreEntryDef* entryDef = ctx.entryDef;
   return upgradeComp.ApplyWeaponUpgrade(entryDef->weaponUpgradeType);
}

bool StoreSystem::TryApplyStatUpgradeReward(const StoreBuyContext& ctx)
{
   UpgradeComponent& upgradeComp = ctx.playerPawn->GetUpgrade();
   const StoreEntryDef* entryDef = ctx.entryDef;
   
   if (entryDef->upgradeLineId == 0)
      return false;
   
   int32 maxLevel = storeEntryTable_->GetMaxLevel(entryDef->upgradeLineId);
   if (maxLevel <= 0)
      return false;
   
   return upgradeComp.ApplyStatUpgrade(entryDef->statUpgradeType, maxLevel);
}
