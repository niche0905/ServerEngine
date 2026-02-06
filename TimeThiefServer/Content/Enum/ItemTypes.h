#pragma once

using ItemId = uint32;

// 최소 아이템 인스턴스 단위 (스택형 아이템 기준)
struct ItemStack
{
    ItemId id{0};
    int32 count{0};

    bool IsValid() const { return id != 0 && count > 0; }
};

enum class InventoryOpCode : uint8
{
    Ok = 0,
    InvalidItem,
    InvalidCount,
    Full,
    NotEnough,
    NotFound,
    Forbidden,
};

enum class ItemChangeReason : uint8
{
    Unknown = 0,
    Loot,
    DropOnDeath,
    Purchase,
    Sell,
    Consume,
    Move,
    System,
};

struct ItemChangeContext
{
    ItemChangeReason reason{ItemChangeReason::Unknown};
    int32 relatedActionId{0};
};

struct InventoryOpResult
{
    InventoryOpCode code{InventoryOpCode::InvalidItem};
    bool accepted{false};

    // 실제 변동량(성공 시 양수/음수)
    ItemStack delta{};

    // (옵션) 슬롯 기반으로 확장하면 slotIndex 같은 걸 추가
};
