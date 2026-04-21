#pragma once
#include "ItemTypes.h"

class ObjectManager;

/*-------------------
   IInventoryOwner
-------------------*/
//
// IInventoryOwner는 인벤토리 소유자가 구현해야 하는 인터페이스입니다.
//

class IInventoryOwner
{
public:
    virtual ~IInventoryOwner() = default;

    virtual int32 GetCapacity() const = 0;
    virtual int32 GetUsedSlots() const = 0;

    virtual int32 GetItemCount(ItemId itemId) const = 0;

    virtual InventoryOpResult AddItem(ItemId itemId, int32 count, const ItemChangeContext& ctx) = 0;
    virtual InventoryOpResult RemoveItem(ItemId itemId, int32 count, const ItemChangeContext& ctx) = 0;

    // 소비(=Remove의 의미 확정 버전)
    virtual InventoryOpResult ConsumeItem(ItemId itemId, int32 count, const ItemChangeContext& ctx) = 0;
};
