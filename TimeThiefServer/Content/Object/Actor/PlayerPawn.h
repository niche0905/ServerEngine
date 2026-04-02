#pragma once
#include "Pawn.h"
#include "Content/Gameplay/Drop/DropOnDeathComponent.h"
#include "Content/Gameplay/Drop/IDropOnDeathOwner.h"
#include "Content/Gameplay/Economy/IWalletOwner.h"
#include "Content/Gameplay/Inventory/IInventoryOwner.h"
#include "Content/Gameplay/Spawn/IRespawnOwner.h"
#include "Content/Gameplay/Inventory/InventoryComponent.h"
#include "Content/Gameplay/Economy/WalletComponent.h"
#include "Content/Gameplay/Spawn/RespawnComponent.h"
#include "Content/Object/Actor/PlayerPawnState.h"

class ObjectManager;

/*--------------
   PlayerPawn
--------------*/
//
// PlayerPawn는 플레이어 캐릭터에 특화된 Pawn 클래스입니다.
// 죽음 시 아이템 드롭, 인벤토리 관리, 지갑(화폐) 관리, 리스폰 메커니즘을 포함합니다.
//

class PlayerPawn : public Pawn
                    , public IDropOnDeathOwner
                    , public IInventoryOwner
                    , public IWalletOwner
                    , public IRespawnOwner
{
public:
    PlayerPawn() = default;
    virtual ~PlayerPawn() = default;
    
    PlayerPawn(const PlayerPawn&) = delete;
    PlayerPawn& operator=(const PlayerPawn&) = delete;
    
// PlayerId (Room 내에서 고유한 플레이어 식별자)
public:
    uint32 GetPlayerId() const { return playerId_; }
    void SetPlayerId(uint32 playerId) { playerId_ = playerId; }
    
// pitch
public:
    float GetPitch() const { return pitch_; }
    void SetPitch(float pitch) { pitch_ = pitch; }
    
// Component
public:
    DropOnDeathComponent& GetDropOnDeath() { return dropOnDeath_; }
    const DropOnDeathComponent& GetDropOnDeath() const { return dropOnDeath_; }
    
    InventoryComponent& GetInventory() override { return inventory_; }
    const InventoryComponent& GetInventory() const { return inventory_; }
    
    WalletComponent& GetWallet() override { return wallet_; }
    const WalletComponent& GetWallet() const { return wallet_; }
    
    RespawnComponent& GetRespawn() { return respawn_; }
    const RespawnComponent& GetRespawn() const { return respawn_; }

// IDropOnDeath
public:
    virtual bool IsConsumable(ItemId itemId) const override;
    virtual bool CanDropOnDeath() const override { return dropOnDeath_.IsEnabled(); }

// IInventoryOwner
public:
    virtual int32 GetCapacity() const override { return inventory_.GetCapacity(); }
    virtual int32 GetUsedSlots() const override { return inventory_.GetUsedSlots(); }

    virtual int32 GetItemCount(ItemId itemId) const override { return inventory_.GetItemCount(itemId); }

    virtual InventoryOpResult AddItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx) override;
    virtual InventoryOpResult RemoveItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx) override;
    virtual InventoryOpResult ConsumeItem(ObjectManager& om, ItemId itemId, int32 count, const ItemChangeContext& ctx) override;
    
// IWalletOwner
public:
    virtual int64 GetBalance(CurrencyId currency) const override { return wallet_.GetBalance(currency); }
    virtual bool CanSpend(CurrencyId currency, int64 amount) const override { return wallet_.CanSpend(currency, amount); }

    virtual MoneyChangeResult AddMoney(ObjectManager& om, CurrencyId currency, int64 amount, const MoneyChangeContext& ctx) override;
    virtual MoneyChangeResult SpendMoney(ObjectManager& om, CurrencyId currency, int64 amount, const MoneyChangeContext& ctx) override;
    
// Respawn
public:
    const Vector3& GetSavedRespawnPosition() const { return respawn_.GetRespawnPosition(); }
    void SetSavedRespawnPosition(const Vector3& pos) { respawn_.SetRespawnPosition(pos); }

// IRespawnOwner 구현
public:
    virtual Vector3 ResolveRespawnPosition(ObjectManager& om) override;
   
    virtual void OnPreRespawn(ObjectManager& om) override;
    virtual void OnPostRespawn(ObjectManager& om) override;
    virtual void ApplyRespawnToWorld(ObjectManager& om, const Vector3& pos) override;
    virtual void GrantSpawnInvulnerability(ObjectManager& om, uint32 durationMs) override;
    
protected:
    void OnSpawn() override;
    void OnPreDestroy() override;
    
    void Tick(float dt) override;
    
protected:
    void OnDeath(ObjectManager& om, const DamageResult& dmgResult) override;
    
    bool ShouldRequestDestroyOnDeath() const override { return false; }
    
private:
    void StartDeadState(ObjectManager& om, const DamageResult& dmgResult);
    
private:
    uint32 playerId_;
    
    float pitch_{0.0f};
    
    DropOnDeathComponent dropOnDeath_{};
    InventoryComponent inventory_{};
    WalletComponent wallet_{};
    RespawnComponent respawn_{};
    
    ActionState actionState_{};
    
};
