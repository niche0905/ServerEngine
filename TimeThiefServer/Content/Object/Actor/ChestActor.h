#pragma once
#include "StaticActor.h"
#include "Content/Gameplay/Inventory/InventoryComponent.h"
#include "Content/Gameplay/Container/IContainerAccess.h"
#include "Content/Gameplay/Drop/IDropOwner.h"
#include "Content/Gameplay/Inventory/IInventoryOwner.h"
#include "Content/Gameplay/Loot/LootSourceComponent.h"

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
{
public:
    ChestActor() = default;
    virtual ~ChestActor() = default;
    
    ChestActor(const ChestActor& Actor) = delete;
    ChestActor& operator=(const ChestActor& Actor) = delete;
    
public:
    ChestKind GetChestKind() const { return chestKind_; }
    void SetChestKind(ChestKind kind) { chestKind_ = kind; }
    
// IDropOwner
public:
    virtual LootBundle GenerateDrops() override;
    
protected:
    void OnSpawn() override;
    void OnPreDestroy() override;
    
public:
    bool CheckOpenPermission(PlayerPawn& byPlayer, int32& outError) const;
    
private:
    ChestKind chestKind_{ChestKind::Normal};
    
    LootSourceComponent lootSource_;
    bool isOpened_{false};
    
    // 변경 전 방식
    // InventoryComponent inventory_;
    // std::unordered_set<ObjectId> openedByPlayers_;
    
};
