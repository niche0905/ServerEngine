#pragma once
#include "EffectTypes.h"

class ObjectManager;

/*-----------
   IEffect
-----------*/
//
// IEffect는 게임 내에서 효과(버프/디버프 등)를 구현하는 인터페이스입니다.
//

class IEffect
{
public:
    virtual ~IEffect() = default;
    
    virtual const EffectDef& GetDef() const = 0;
    
    virtual void OnApply(ObjectManager& om, ObjectId target, const EffectApplyContext& ctx) = 0;
    virtual void OnRefresh(ObjectManager& om, ObjectId target, const EffectApplyContext& ctx) = 0;
    virtual void OnRemove(ObjectManager& om, ObjectId target, uint64 nowMs) = 0;
    
    virtual void OnExpired(ObjectManager& om, ObjectId target, uint64 nowMs)
    {
        (void)om; (void)target; (void)nowMs;
        // 기본 구현은 아무 것도 하지 않음
    }
    
};
