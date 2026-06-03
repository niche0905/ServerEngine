#pragma once
#include <algorithm>
#include "Pawn.h"
#include "Content/Gameplay/Combat/PlayerCombatComponent.h"
#include "Content/Gameplay/Economy/IWalletOwner.h"
#include "Content/Gameplay/Inventory/IInventoryOwner.h"
#include "Content/Gameplay/Inventory/InventoryComponent.h"
#include "Content/Gameplay/Economy/WalletComponent.h"
#include "Content/Gameplay/Save/SaveComponent.h"
#include "Content/Gameplay/Skill/SkillComponent.h"
#include "Content/Gameplay/Upgrade/UpgradeComponent.h"
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
                    , public IInventoryOwner
                    , public IWalletOwner
{
public:
    PlayerPawn() = default;
    PlayerPawn(int32 initialTimePoint, int32 zoneDamageTimePointMultiplier)
        : initialTimePoint_(initialTimePoint)
        , zoneDamageTimePointMultiplier_(std::max(1, zoneDamageTimePointMultiplier))
    {
    }
    virtual ~PlayerPawn() = default;
    
    PlayerPawn(const PlayerPawn&) = delete;
    PlayerPawn& operator=(const PlayerPawn&) = delete;
    
public:
   virtual se::common::ObjectType GetObjectType() const override { return se::common::OBJ_PLAYER; }
    
// PlayerId (Room 내에서 고유한 플레이어 식별자)
public:
    virtual PlayerId GetOwnerPlayerId() const override { return playerId_; }
    
    void SetOwnerPlayerId(PlayerId playerId) { playerId_ = playerId; }
    void SetRespawnCostTimePoint(int32 cost) { respawnCostTimePoint_ = cost; }
    
protected:
    virtual int32 ResolveIncomingDamage(int32 amount, const DamageContext& ctx) override;
    
public:
    float GetAimYaw() const { return aimYaw_; }
    void SetAimYaw(float aimYaw) { aimYaw_ = aimYaw; }
    
    float GetPitch() const { return pitch_; }
    void SetPitch(float pitch) { pitch_ = pitch; }
    
    int32 GetMovementMode() const { return movementMode_; }
    void SetMovementMode(int32 movementMode) { movementMode_ = movementMode; }
    
    int32 GetSpeed() const { return speed_; }
    
// Component
public:
    PlayerCombatComponent* GetPlayerCombat() { return static_cast<PlayerCombatComponent*>(combat_.get()); }
    const PlayerCombatComponent* GetPlayerCombat() const { return static_cast<const PlayerCombatComponent*>(combat_.get()); }
    
    InventoryComponent& GetInventory() { return inventory_; }
    const InventoryComponent& GetInventory() const { return inventory_; }
    
    WalletComponent& GetWallet() { return wallet_; }
    const WalletComponent& GetWallet() const { return wallet_; }
    
    SkillComponent& GetSkill() { return skill_; }
    const SkillComponent& GetSkill() const { return skill_; }
    
    UpgradeComponent& GetUpgrade() { return upgrade_; }
    const UpgradeComponent& GetUpgrade() const { return upgrade_; }
    
    SaveComponent& GetSave() { return save_; }
    const SaveComponent& GetSave() const { return save_; }
    
public:
    virtual void Damaged(const DamageResult& dmgResult) override;
    
    void Heal(int32 amount);
    
// Respawn
public:
    virtual void OnPreRespawn(ObjectManager& om) override;
    virtual void OnPostRespawn(ObjectManager& om) override;
    
    virtual bool TryReserveRespawn() override;

// IDropOwner
public:
    virtual LootBundle GenerateDrops() override;

// IInventoryOwner
public:
    virtual int32 GetCapacity() const override { return inventory_.GetCapacity(); }
    virtual int32 GetUsedSlots() const override { return inventory_.GetUsedSlots(); }

    virtual int32 GetItemCount(ItemId itemId) const override { return inventory_.GetItemCount(itemId); }

    virtual InventoryOpResult AddItem(ItemId itemId, int32 count, const ItemChangeContext& ctx) override;
    virtual InventoryOpResult RemoveItem(ItemId itemId, int32 count, const ItemChangeContext& ctx) override;
    virtual InventoryOpResult ConsumeItem(ItemId itemId, int32 count, const ItemChangeContext& ctx) override;
    
// IWalletOwner
public:
    virtual CurrencyAmount GetBalance(CurrencyType currency) const override { return wallet_.GetBalance(currency); }
    virtual bool CanSpend(CurrencyType currency, CurrencyAmount amount) const override { return wallet_.CanSpend(currency, amount); }

    virtual MoneyChangeResult AddMoney(CurrencyType currency, CurrencyAmount amount, const MoneyChangeContext& ctx) override;
    virtual MoneyChangeResult SpendMoney(CurrencyType currency, CurrencyAmount amount, const MoneyChangeContext& ctx) override;
    
// Player State (Action State)
public:
    bool IsAiming() const { return actionState_.isAiming; }
    void SetAiming(bool isAiming) { actionState_.isAiming = isAiming; }
    
    bool IsJumping() const { return actionState_.isJumping; }
    void SetJumping(bool isJumping) { actionState_.isJumping = isJumping; }
    
    bool IsDoubleJumping() const { return actionState_.isDoubleJumping; }
    void SetDoubleJumping(bool isDoubleJumping) { actionState_.isDoubleJumping = isDoubleJumping; }
    
    bool IsCrouching() const { return actionState_.isCrouching; }
    void SetCrouching(bool isCrouching) { actionState_.isCrouching = isCrouching; }
    
    bool IsWired() const { return actionState_.isWireActing; }
    void SetWired(bool isWired) { actionState_.isWireActing = isWired; }
    
// Skill
public:
    void OnSkillChanged(SkillId skillId);
    
// Weapon
public:
    void OnWeaponUpgradeApplied(WeaponUpgradeCode code);
    void RefreshWeaponStatsByUpgrade(uint32 code);
    void RefreshWeaponStats();
    
// Stat
public:
    void OnStatUpgradeApplied(StatUpgradeCode code, int32 newLevel);
    
public:
    bool TrySetSavePoint(const Vector3& location);
    
protected:
    void OnSpawn() override;
    void OnPreDestroy() override;
    
    void Tick(float dt) override;
    
protected:
    virtual void OnDeath(ObjectManager& om, const DamageContext& ctx, const DamageResult& dmgResult) override;
    
    bool ShouldRequestDestroyOnDeath() const override { return false; }
    
private:
    void StartDeadState(ObjectManager& om, const DamageResult& dmgResult);
    
private:
    void InitDefaultLoadout();
    
private:
    PlayerId                    playerId_;
    
    float                       aimYaw_{0.0f};
    float                       pitch_{0.0f};
    int32                       movementMode_{0};
    uint32                      deathCount_{0};
    
    InventoryComponent          inventory_{};
    WalletComponent             wallet_{};
    SkillComponent              skill_{};
    UpgradeComponent            upgrade_{};
    SaveComponent               save_{};
    
    int32                       speed_{};
    int32                       respawnCostTimePoint_{100};
    int32                       initialTimePoint_{1000};
    int32                       zoneDamageTimePointMultiplier_{10};
    
    ActionState                 actionState_{};
    
};
