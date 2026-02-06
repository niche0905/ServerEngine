#pragma once
#include "BaseComponent.h"
#include "Content/Object/ObjectId.h"
#include "Content/Enum/ItemTypes.h"

class ObjectManager;
struct ItemStack;

/*----------------------
   InventoryComponent
----------------------*/
//
// InventoryComponent는 플레이어 혹은 NPC가 소유한 아이템들을 관리합니다.
//

class InventoryComponent : public BaseComponent
{
public:
   void Init(ObjectId owner, int32 capacity)
   {
      SetOwner(owner);
      
      SetCapacity(capacity);
   }
   
   void SetCapacity(int32 capacity)
   {
      capacity_ = std::max(0, capacity);
      slots_.clear();
      slots_.resize(static_cast<size_t>(capacity_));
   }
   
   int32 GetCapacity() const { return capacity_; }
   
   int32 GetUsedSlots() const
   {
      int32 used = 0;
      for (const auto& slot : slots_) {
         if (slot.IsValid()) {
            ++used;
         }
      }
      
      return used;
   }
   
   const std::vector<ItemStack>& GetSlots() const { return slots_; }
   
   int32 GetItemCount(ItemId itemId) const
   {
      if (itemId == 0) return 0;
      
      int32 totalCount = 0;
      for (const auto& slot : slots_) {
         if (slot.id == itemId) {
            totalCount += slot.count;
         }
      }
      
      return totalCount;
   }
   
   bool HasItem(ItemId itemId, int32 count) const
   {
      if (itemId == 0 or count <= 0) return false;
      return GetItemCount(itemId) >= count;
   }
   
   InventoryOpResult AddItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx)
   {
      InventoryOpResult result;
      if (itemId == 0) { result.code = InventoryOpCode::InvalidItem; return result; }
      if (count <= 0) { result.code = InventoryOpCode::InvalidCount; return result; }
      
      for (auto& slot : slots_) {
         if (slot.id == itemId and slot.count > 0) {
            slot.count += count;
            
            result.code = InventoryOpCode::Ok;
            result.delta = ItemStack{ itemId, count };
            
            result.accepted = true;

            return result;
         }
      }
      
      for (auto& slot : slots_) {
         if (not slot.IsValid()) {
            slot.id = itemId;
            slot.count = count;
            
            result.code = InventoryOpCode::Ok;
            result.delta = ItemStack{ itemId, count };
            
            result.accepted = true;

            return result;
         }
      }
      
      result.code = InventoryOpCode::Full;
      return result;
   }
   
   InventoryOpResult RemoveItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx)
   {
      InventoryOpResult result;
      if (itemId == 0) { result.code = InventoryOpCode::InvalidItem; return result; }
      if (count <= 0) { result.code = InventoryOpCode::InvalidCount; return result; }
      
      if (not HasItem(itemId, count)) {
         result.code = InventoryOpCode::NotEnough;
         return result;
      }
      
      int32 remaining = count;
      
      for (auto& slot : slots_) {
         if (slot.id != itemId or slot.count <= 0) continue;
         if (remaining <= 0) break;
         
         const int32 take = std::min(slot.count, remaining);
         slot.count -= take;
         remaining -= take;
         
         if (slot.count == 0) 
            slot.id = 0;
      }
      
      result.code = InventoryOpCode::Ok;
      result.delta = ItemStack{ itemId, -count };
      
      result.accepted = true;
      
      return result;
   }
   
   InventoryOpResult ConsumeItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx)
   {
      return RemoveItem(om, itemId, count, ctx);
   }
   
   void Clear()
   {
      for (auto& slot : slots_) {
         slot = ItemStack{};
      }
   }
   
private:
   int32 capacity_{ 0 };
   std::vector<ItemStack> slots_;
   
};