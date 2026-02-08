#pragma once
#include "Content/Shared/BaseComponent.h"
#include "Content/Object/BaseObject.h"
#include "DropTypes.h"
#include "IDropOnDeathOwner.h"
#include "Content/Gameplay/Inventory/ItemTypes.h"
#include "Content/Gameplay/Inventory/InventoryComponent.h"
#include "Content/Gameplay/Economy/WalletTypes.h"
#include "Content/Gameplay/Economy/WalletComponent.h"
#include "Content/Gameplay/Loot/LootTypes.h"
#include <unordered_map>


class ObjectManager;

struct DropOnDeathContext
{
   ObjectId owner{};
   ObjectId killer{};
   uint64 nowMs{0};
   DropReason reason{DropReason::Death};
};

struct DropOnDeathResult
{
   bool generated{false};
   LootBundle bundle;
   DropSpawnPolicy spawnPolicy;
};

/*------------------------
   DropOnDeathComponent
------------------------*/
//
// DropOnDeathComponent는 객체가 죽을 때 아이템이나 화폐를 드롭하는 기능을 담당하는 컴포넌트입니다.
//

class DropOnDeathComponent : public BaseComponent
{
public:
   void Init(ObjectId owner, const DropOnDeathPolicy& policy)
   {
      SetOwner(owner);
      
      policy_ = policy;
      enabled_ = true;
   }
   
   void SetEnabled(bool enable) { enabled_ = enable; }
   bool IsEnabled() const { return enabled_; }
   
   const DropOnDeathPolicy& GetPolicy() const { return policy_; }
   void SetPolicy(const DropOnDeathPolicy& policy) { policy_ = policy; }
   
   DropOnDeathResult Generate(ObjectManager& om, IDropOnDeathOwner& owner, const DropOnDeathContext& ctx)
   {
      (void)om;      // 아직 ObjectManager를 사용하지 않음 (나중에 사용할 수도 있기에...)
      
      DropOnDeathResult result;
      result.spawnPolicy.mode = policy_.mode;
      result.spawnPolicy.maxScatterItemSpawns = policy_.maxScatterItemSpawns;
      result.spawnPolicy.scatterRadius = policy_.scatterRadius;
      
      if (not enabled_) return result;
      if (not owner.CanDropOnDeath()) return result;
      
      // 화폐 드롭 처리
      if (policy_.moneyDropRate > 0.0f) {
         // 화폐 드롭 로직
         // 우선은 TimePoint만 사용한다고 가정
         const CurrencyId timePoint = static_cast<CurrencyId>(CurrencyType::TimePoint);
         
         const int64 before = owner.GetWallet().GetBalance(timePoint);
         if (before > 0) {
            // const double rate = std::max(0.0, std::min(1.0, static_cast<double>(policy_.moneyDropRate)));
            const double rate = std::clamp(static_cast<double>(policy_.moneyDropRate), 0.0, 1.0);
            const int64 dropAmount = static_cast<int64>(std::floor(static_cast<double>(before) * rate));
            if (dropAmount > 0) {
               MoneyChangeContext mctx{};
               mctx.reason = MoneyChangeReason::DropOnDeath;
               
               auto spend = owner.GetWallet().SpendMoney(om, timePoint, dropAmount, mctx);
               if (spend.accepted)
                  result.bundle.AddMoney(timePoint, dropAmount);
            }
         }
      }
      
      // 아이템 드롭 처리
      auto& inventory = owner.GetInventory();
      const auto& slots = inventory.GetSlots();
      
      std::unordered_map<ItemId, int32> totals;
      totals.reserve(slots.size());
      
      for (const auto& slot : slots) {
         if (not slot.IsValid()) continue;
         
         const bool isCons = owner.IsConsumable(slot.id);
         
         if (isCons and not policy_.dropConsumables) continue;
         
         totals[slot.id] += slot.count;
      }
      
      if (not totals.empty()) {
         ItemChangeContext ictx{};
         ictx = ItemChangeReason::DropOnDeath;
         
         for (auto& [itemId, count] : totals) {
            if (itemId == 0 or count <= 0) continue;
            
            auto rem = inventory.RemoveItem(om, itemId, count, ictx);
            if (rem.accepted) {
               const int32 applied = -rem.delta.count;
               if (applied > 0) {
                  result.bundle.AddItem(itemId, applied);
               }
            }
         }
      }
      
      result.generated = not result.bundle.Empty();
      
      return result;
   }
   
private:
   bool enabled_{true};
   DropOnDeathPolicy policy_;
   
};
