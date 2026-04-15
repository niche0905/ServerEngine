#pragma once
#include "Actor.h"
#include "Content/Gameplay/Combat/IDamageable.h"
#include "Content/Gameplay/Combat/HealthComponent.h"
#include "Content/Gameplay/Combat/CombatComponent.h"
#include "Content/Gameplay/Cooldown/CooldownComponent.h"
#include "Content/Gameplay/Effects/EffectComponent.h"
#include "Content/Gameplay/Effects/IEffectOwner.h"
#include "Content/Gameplay/Spawn/IRespawnOwner.h"
#include "Content/Gameplay/Spawn/RespawnComponent.h"

/*--------
   Pawn
--------*/
//
// Pawn은 액터의 하위 클래스이며 움직이는 생명체에 대한 기본 기능을 제공합니다.
// IDamageable 인터페이스를 구현하여 피해를 받을 수 있는 기능을 포함합니다.
// Cooldown과 Effect 컴포넌트를 포함하여 게임플레이 메커니즘을 지원합니다.
//

class CombatComponent;

class Pawn : public Actor
             , public IDamageable
             , public IEffectOwner
             , public IRespawnOwner
{
public:
    Pawn() = default;
    virtual ~Pawn() = default;
    
    Pawn(const Pawn&) = delete;
    Pawn& operator=(const Pawn&) = delete;
    
public:
    virtual PlayerId GetOwnerPlayerId() const { return 0;}
    
    bool IsOwnedByPlayer() const { return GetOwnerPlayerId() != 0; }
    
// Health
public:
    HealthComponent& GetHealth() { return health_; }
    const HealthComponent& GetHealth() const { return health_; }
    
    bool IsDead() const { return isDead_; }
    
// Movement
public:
    const Vector3& GetVelocity() const { return velocity_; }
    void SetVelocity(const Vector3& velocity);
    
    virtual void IntegrateMove(float dt);  // 단순 이동 (물리/충돌 미적용 <- 상속받아서 구현 필요)
    
// Damageable
public:
    virtual DamageResult ApplyDamage(ObjectManager& om, int32 amount, const DamageContext& ctx) override;
    virtual void Damaged(const DamageResult& dmgResult);
    
    virtual bool IsHpAlive() const override;
    virtual int32 GetHp() const override;
    virtual int32 GetMaxHp() const override;
    
protected:
    virtual int32 ModifyIncomingDamage(int32 amount, const DamageContext& ctx) { return amount; }
    virtual void OnBeforeApplyDamage(const DamageContext& ctx, int32& amount) {}
    virtual void OnAfterApplyDamage(const DamageResult& dmgResult, const DamageContext& ctx) {}
    
// Cooldown
public:
    CooldownComponent& GetCooldowns() { return cooldowns_; }
    const CooldownComponent& GetCooldowns() const { return cooldowns_; }
    
// Effect
public:
    virtual EffectComponent& GetEffectComponent() override { return effects_; }
    const EffectComponent& GetEffectComponent() const { return effects_; }
    
    virtual HealthComponent* TryGetHealth() override { return &health_; }
    
    virtual void OnEffectChanged() override {}
    
// Respawn
public:
    RespawnComponent& GetRespawnComponent() { return respawn_; }
    const RespawnComponent& GetRespawnComponent() const { return respawn_; }
    
    virtual SE::Math::Vector3 ResolveRespawnPosition(ObjectManager& om) override;
    
    virtual void OnPreRespawn(ObjectManager& om) override;
    virtual void OnPostRespawn(ObjectManager& om) override;
    virtual void ApplyRespawnToWorld(ObjectManager& om, const SE::Math::Vector3& pos) override;
    virtual void GrantSpawnInvulnerability(ObjectManager& om, uint32 durationMs) override;
    
    const Vector3& GetSavedRespawnPosition() const { return respawn_.GetRespawnPosition(); }
    void SetSavedRespawnPosition(const Vector3& pos) { respawn_.SetRespawnPosition(pos); }
    
// Combat
public:
        CombatComponent* GetCombatComponent() { return combat_.get(); }
        const CombatComponent* GetCombatComponent() const { return combat_.get(); }
    
// Lifecycle
protected:
    void OnSpawn() override;
    void OnPreDestroy() override;
    
    void Tick(float dt) override;
    
protected:
    virtual void OnDeath(ObjectManager& om, const DamageResult& dmgResult);
    
    // 죽었을 때 즉시 파괴 요청을 할지 여부
    virtual bool ShouldRequestDestroyOnDeath() const { return false; }
    
protected:
    void SetDead(bool isDead) { isDead_ = isDead; }
    
protected:
    HealthComponent                     health_;
    CooldownComponent                   cooldowns_;
    EffectComponent                     effects_;
    RespawnComponent                    respawn_;
    std::unique_ptr<CombatComponent>    combat_;
    
    Vector3                             velocity_{};
    bool                                isDead_{false};
    
};
