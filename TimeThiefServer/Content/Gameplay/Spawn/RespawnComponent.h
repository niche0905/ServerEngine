#pragma once
#include "IRespawnOwner.h"
#include "Content/Shared/BaseComponent.h"
#include "Content/Object/BaseObject.h"
#include "RespawnTypes.h"

class ObjectManager;
struct SE::Math::Vector3;
class IRespawnOwner;

/*--------------------
   RespawnComponent
--------------------*/
//
// RespawnComponent는 오브젝트의 리스폰 상태와 정책을 관리하는 컴포넌트입니다.
//

class RespawnComponent : public BaseComponent
{
public:
    using Vector3 = SE::Math::Vector3;
    
public:
    void Init(ObjectId owner, const RespawnPolicy& policy)
    {
        SetOwner(owner);
        
        policy_ = policy;
        state_ = RespawnState::Alive;
        
        scheduledAtMs_ = 0;
        deadSinceMs_ = 0;
    }
    
    const RespawnPolicy& GetPolicy() const { return policy_; }
    void SetPolicy(const RespawnPolicy& policy) { policy_ = policy; }
    
    RespawnState GetState() const { return state_; }
    bool IsEnabled() const { return policy_.enabled; }
    
    const Vector3& GetRespawnPosition() const { return respawnPosition_; }
    void SetRespawnPosition(const Vector3& pos) { respawnPosition_ = pos; }
    
    // 사망 시 호출
    bool OnDeath(uint64 nowMs)
    {
        if (not IsEnabled()) { state_ = RespawnState::Dead; return false; }
        
        state_ = RespawnState::Scheduled;
        deadSinceMs_ = nowMs;
        
        scheduledAtMs_ = nowMs + static_cast<uint64>(policy_.delayMs);
        state_ = RespawnState::Scheduled;
        
        return true;
    }
    
    // 리스폰 예약 처리 (수동)
    bool Schedule(uint64 nowMs, uint32 delayMs)
    {
        if (not IsEnabled()) return false;
        
        deadSinceMs_ = nowMs;
        
        scheduledAtMs_ = nowMs + static_cast<uint64>(delayMs);
        state_ = RespawnState::Scheduled;
        
        return true;
    }
    
    bool IsScheduled() const { return state_ == RespawnState::Scheduled; }
    uint64 GetScheduledAtMs() const { return scheduledAtMs_; }
    
    // 시간이 되었는 지 확인
    bool IsDue(uint64 nowMs) const
    {
        return (IsScheduled() && scheduledAtMs_ <= nowMs);
    }
    
    bool ExecuteRespawn(ObjectManager& om, IRespawnOwner& owner, const RespawnContext& ctx)
    {
        if (not IsEnabled()) return false;
        if (not IsScheduled()) return false;
        if (ctx.nowMs < scheduledAtMs_) return false;
        
        state_ = RespawnState::Respawning;
        
        owner.OnPreRespawn(om);
        owner.ApplyRespawnToWorld(om, respawnPosition_);
        owner.OnPostRespawn(om);
        
        if (policy_.invulMs > 0) {
            owner.GrantSpawnInvulnerability(om, policy_.invulMs);
        }
        
        scheduledAtMs_ = 0;
        state_ = RespawnState::Alive;
        
        return true;
    }
    
    void DisableRespawn()
    {
        policy_.enabled = false;
        state_ = RespawnState::Dead;
        scheduledAtMs_ = 0;
    }
    
private:
    RespawnPolicy policy_{};
    
    RespawnState state_{RespawnState::None};
    
    uint64 deadSinceMs_{0};
    uint64 scheduledAtMs_{0};
    
    Vector3 respawnPosition_{0.0f, 0.0f, 0.0f};
    
};