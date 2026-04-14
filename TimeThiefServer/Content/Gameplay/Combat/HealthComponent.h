#pragma once
#include "Content/Shared/BaseComponent.h"
#include "Content/Object/ObjectId.h"
#include "DamageTypes.h"
#include <algorithm>

class ObjectManager;

/*------------------
   HeathComponent
------------------*/
//
// HeathComponent는 객체의 체력을 관리하는 컴포넌트입니다.
//

class HealthComponent : public BaseComponent
{
public:
    void Init(ObjectId owner, int32 maxHp, int32 startHp = -1)
    {
        SetOwner(owner);
        
        maxHp_ = std::max(1, maxHp);
        
        if (startHp < 0) hp_ = maxHp_;
        else hp_ = std::clamp(startHp, 0, maxHp_);
        
        invincible_ = false;
    }
    
    // Getter
    int32 GetHp() const { return hp_; }
    int32 GetMaxHp() const { return maxHp_; }
    bool IsAlive() const { return hp_ > 0; }
    
    // Invincibility
    void SetInvincible(bool v) { invincible_ = v; }
    bool IsInvincible() const { return invincible_; }

    // Damage & Heal Game play
    DamageResult ApplyDamage(int32 amount, const DamageContext& damageContext)
    {
        DamageResult result{};
        result.requested = amount;
        
        if (amount <= 0) return result;
        if (hp_ <= 0) return result;
        if (invincible_) return result;
        
        result.accepted = true;
        result.hpBefore = hp_;
        
        const int32 newHp = std::max(0, hp_ - amount);
        hp_ = newHp;
        
        result.hpAfter = hp_;
        result.applied = result.hpBefore - result.hpAfter;
        result.killed = (hp_ == 0);
        
        return result;
    }
    
    int32 Heal(int32 amount)
    {
        if (amount <= 0 or hp_ <= 0) return 0;

        const int32 before = hp_;
        hp_ = std::min(maxHp_, hp_ + amount);
  
        return hp_ - before;
    }
    
    void Revive(int32 hp)
    {
        hp_ = std::clamp(hp, 1, maxHp_);
        invincible_ = false;
    }
    
    // 디버깅 용도
    void SetHpUnsafe(int32 hp)
    {
        hp_ = std::clamp(hp, 0, maxHp_);
    }
    
private:
    int32 hp_{0};
    int32 maxHp_{0};
    bool invincible_{false};
    
};