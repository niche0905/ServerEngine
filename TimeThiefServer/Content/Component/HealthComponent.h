#pragma once
#include "BaseComponent.h"
#include "Content/Object/ObjectId.h"

struct DamageContext
{
    ObjectId attacker{};
    int32 skillId{0};       // TODO: 나중에 스킬 시스템으로 확장
};

struct DamageResult
{
    int32 applied{0};
    bool killed{false};
};

class HeathComponent : public BaseComponent
{
public:
    void Init(ObjectId owner, int32 maxHp)
    {
        owner_ = owner;
        maxHp_ = maxHp;
        hp_ = maxHp;
    }
    
    int32 GetHp() const { return hp_; }
    int32 GetMaxHp() const { return maxHp_; }
    bool IsAlive() const { return hp_ > 0; }
    
    DamageResult ApplyDamage(int32 amount, const DamageContext& damageContext)
    {
        DamageResult result{};
        if (amount <= 0 or hp_ <= 0) return result;
        
        const int32 before = hp_;
        hp_ -= amount;
        if (hp_ < 0) hp_ = 0;
        
        result.applied = before - hp_;
        result.killed = (hp_ == 0);
        
        return result;
    }
    
    int32 Heal(int32 amount)
    {
        if (amount <= 0 or hp_ <= 0) return 0;

        const int32 before = hp_;
        hp_ += amount;
        if (hp_ > maxHp_) hp_ = maxHp_;
  
        return hp_ - before;
    }
    
private:
    int32 hp_{0};
    int32 maxHp_{0};
    
};