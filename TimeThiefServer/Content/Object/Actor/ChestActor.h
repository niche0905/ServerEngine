#pragma once
#include "StaticActor.h"
#include "Content/Gameplay/Inventory/InventoryComponent.h"
#include "Content/Gameplay/Container/IContainerAccess.h"
#include "Content/Gameplay/Drop/IDropOwner.h"
#include "Content/Gameplay/Inventory/IInventoryOwner.h"

class ObjectManager;
class PlayerPawn;

enum class ChestKind : uint8
{
    Normal = 0,     // 일반 상자 (맵에 배치되는 상자)
    CorpseBox,      // 시체 상자 (플레이어가 죽으면 생성되는 상자)
};

/*--------------
   ChestActor
--------------*/
//
// ChestActor는 상자(Chest) 역할을 하는 액터입니다.
//

class ChestActor : public StaticActor
                    , public IDropOwner
                   , public IContainerAccess
                   , public IInventoryOwner
{
public:
    ChestActor() = default;
    virtual ~ChestActor() = default;
    
    ChestActor(const ChestActor& Actor) = delete;
    ChestActor& operator=(const ChestActor& Actor) = delete;
    
public:
    ChestKind GetChestKind() const { return chestKind_; }
    void SetChestKind(ChestKind kind) { chestKind_ = kind; }
    
    InventoryComponent& GetInventory() { return inventory_; }
    const InventoryComponent& GetInventory() const { return inventory_; }
    
// IDropOwner
public:
    virtual LootBundle GenerateDrops() override;
    
// IContainerAccess
public:
    ContainerOpenResult TryOpen(ObjectManager& om, PlayerPawn& byPlayer) override;
    void Close(ObjectManager& om, PlayerPawn& byPlayer) override;
    
    ContainerTakeResult TryTakeFromContainer(ObjectManager& om, PlayerPawn& byPlayer, int32 containerSlot, int32 takeCount) override;
    ContainerTakeResult TryTakeAll(ObjectManager& om, PlayerPawn& byPlayer) override;
    
    bool IsEmpty() const override;
    
// IInventoryOwner
public:
    virtual int32 GetCapacity() const override;
    virtual int32 GetUsedSlots() const override;

    virtual int32 GetItemCount(ItemId itemId) const override;

    virtual InventoryOpResult AddItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx) override;
    virtual InventoryOpResult RemoveItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx) override;
    virtual InventoryOpResult ConsumeItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx) override;
    
protected:
    void OnSpawn() override;
    void OnPreDestroy() override;
    
private:
    bool CheckOpenPermission(ObjectManager& om, PlayerPawn& byPlayer, int32& outError) const;
    
    
private:
    ChestKind chestKind_{ChestKind::Normal};
    
    InventoryComponent inventory_;
    
    std::unordered_set<ObjectId> openedByPlayers_;
    
};
