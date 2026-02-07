#pragma once
#include "Content/Shared/BaseComponent.h"
#include "Content/Object/ObjectId.h"
#include "InteractTypes.h"

class ObjectManager;

/*---------------------
   InteractComponent
---------------------*/
//
// InteractComponent는 객체의 상호작용 상태를 관리하는 컴포넌트입니다.
// enabled/locked/busy 3개의 상태와 cooldown(서버 시간 기반)을 지원합니다.
//

class InteractComponent : public BaseComponent
{
public:
    void Init(ObjectId owner)
    {
        SetOwner(owner);
        
        enabled_ = true;
        locked_ = false;
        busy_ = false;
        
        cooldownMs_ = 0;
        lastInteractAtMs_ = 0;
    }

    // --- state flags ---
    void SetEnabled(bool v) { enabled_ = v; }
    bool IsEnabled() const { return enabled_; }

    void SetLocked(bool v) { locked_ = v; }
    bool IsLocked() const { return locked_; }

    void SetBusy(bool v) { busy_ = v; }
    bool IsBusy() const { return busy_; }

    // --- cooldown ---
    void SetCooldownMs(uint32 ms) { cooldownMs_ = ms; }
    uint32 GetCooldownMs() const { return cooldownMs_; }

    bool IsOnCooldown(uint64 nowMs) const
    {
        if (cooldownMs_ == 0) return false;
        return (nowMs < lastInteractAtMs_ + static_cast<uint64>(cooldownMs_));
    }

    uint64 GetLastInteractAtMs() const { return lastInteractAtMs_; }

    // --- validation (공통) ---
    InteractResultCode CanInteractCommon(uint64 nowMs) const
    {
        if (not enabled_) return InteractResultCode::NotInteractable;
        if (busy_)     return InteractResultCode::Busy;
        if (locked_)   return InteractResultCode::Locked;
        if (IsOnCooldown(nowMs)) return InteractResultCode::Cooldown;
        return InteractResultCode::Ok;
    }

    // --- commit ---
    void CommitInteract(uint64 nowMs)
    {
        lastInteractAtMs_ = nowMs;
    }

private:
    bool enabled_{true};
    bool locked_{false};
    bool busy_{false};

    uint32 cooldownMs_{0};
    uint64 lastInteractAtMs_{0};
    
};
