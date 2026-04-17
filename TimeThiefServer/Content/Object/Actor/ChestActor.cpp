#include "pch.h"
#include "ChestActor.h"
#include "PlayerPawn.h"

/*--------------
   ChestActor
--------------*/

LootBundle ChestActor::GenerateDrops()
{
    // TODO: Loot 이용해서 처리 해도 되고
    //       Init 시점에서 미리 아이템을 채워 놓는 방식으로 해도 될 듯
    
     return LootBundle{};
}

ContainerOpenResult ChestActor::TryOpen(ObjectManager& om, PlayerPawn& byPlayer)
{
    ContainerOpenResult result;
    int32 err = 0;
    if (not CheckOpenPermission(om, byPlayer, err)) {
        result.ok = false;
        result.errorCode = err;
        return result;
    }
    
    openedByPlayers_.insert(byPlayer.GetId());
    result.ok = true;
    return result;
}

void ChestActor::Close(ObjectManager& om, PlayerPawn& byPlayer)
{
    (void)om;
    
    if (openedByPlayers_.contains(byPlayer.GetId()))
        openedByPlayers_.erase(byPlayer.GetId());
}

ContainerTakeResult ChestActor::TryTakeFromContainer(ObjectManager& om, PlayerPawn& byPlayer, int32 containerSlot,
    int32 takeCount)
{
    ContainerTakeResult result;
    int32 err = 0;
    if (not CheckOpenPermission(om, byPlayer, err)) {
        result.ok = false;
        result.errorCode = err;
        return result;
    }
    
    if (takeCount <= 0) {
        result.ok = false;
        result.errorCode = 2;   // 잘못된 개수
        return result;
    }
    
    InventoryOpResult moveResult = inventory_.RemoveItemSlot(om, containerSlot, takeCount, ItemChangeContext{ ItemChangeReason::Loot, 0 });
    // TODO: Player에게 아이템 추가 시도 및 통지
    // byPlayer.AddItem(om, moveResult);
    
    const int32 moved = moveResult.accepted ? -moveResult.delta.count : 0;
    if (moved <= 0) {
        result.ok = false;
        result.errorCode = 3;   // 아이템 이동 실패
        return result;
    }
    
    result.ok = true;
    result.moveCount = moved;
    
    return result;
}

ContainerTakeResult ChestActor::TryTakeAll(ObjectManager& om, PlayerPawn& byPlayer)
{
    ContainerTakeResult result;
    
    int32 err = 0;
    
    if (not CheckOpenPermission(om, byPlayer, err)) {
        result.ok = false;
        result.errorCode = err;
        return result;
    }
    
    int32 totalMoved = 0;
    for (int32 slotIndex = 0; slotIndex < GetCapacity(); ++slotIndex)
    {
        auto& slot = inventory_.GetSlots()[static_cast<size_t>(slotIndex)];
        if (not slot.IsValid()) continue;
        
        InventoryOpResult moveResult = inventory_.RemoveItemSlot(om, slotIndex, slot.count, ItemChangeContext{ ItemChangeReason::Loot, 0 });
        // TODO: Player에게 아이템 추가 시도 및 통지
        // byPlayer.AddItem(om, moveResult);
        
        const int32 moved = moveResult.accepted ? -moveResult.delta.count : 0;
        totalMoved += moved;
    }
    
    if (totalMoved <= 0) {
        result.ok = false;
        result.errorCode = 3;   // 아이템 이동 실패
        return result;
    }
    
    result.ok = true;
    result.moveCount = totalMoved;
    
    return result;
}

bool ChestActor::IsEmpty() const
{
    return inventory_.GetUsedSlots() == 0;
}

int32 ChestActor::GetCapacity() const
{
    return inventory_.GetCapacity();
}

int32 ChestActor::GetUsedSlots() const
{
    return inventory_.GetUsedSlots();
}

int32 ChestActor::GetItemCount(ItemId itemId) const
{
    return inventory_.GetItemCount(itemId);
}

InventoryOpResult ChestActor::AddItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx)
{
    return inventory_.AddItem(om, itemId, count, ctx);
}

InventoryOpResult ChestActor::RemoveItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx)
{
    return inventory_.RemoveItem(om, itemId, count, ctx);
}

InventoryOpResult ChestActor::ConsumeItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx)
{
    return inventory_.ConsumeItem(om, itemId, count, ctx);
}

void ChestActor::OnSpawn()
{
    StaticActor::OnSpawn();
    
    openedByPlayers_.reserve(8);    // 최대 플레이어 수 8명
}

void ChestActor::OnPreDestroy()
{
    StaticActor::OnPreDestroy();
}

bool ChestActor::CheckOpenPermission(ObjectManager& om, PlayerPawn& byPlayer, int32& outError) const
{
    (void)om;
    
    // TODO: 거리 체크 / 시야 체크 / 상태 체크
    (void)byPlayer;
    
    // 오픈에 성공
    outError = 0;
    return true;
}
