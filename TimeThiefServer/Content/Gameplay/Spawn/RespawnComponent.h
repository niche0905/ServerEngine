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
        
        deadSinceMs_ = 0;
    }
    
    const RespawnPolicy& GetPolicy() const { return policy_; }
    void SetPolicy(const RespawnPolicy& policy) { policy_ = policy; }
    
    RespawnState GetState() const { return state_; }
    bool IsScheduled() const { return state_ == RespawnState::Scheduled; }
    bool IsEnabled() const { return policy_.enabled; }
    
    const Vector3& GetRespawnPosition() const { return respawnPosition_; }
    void SetRespawnPosition(const Vector3& pos) { respawnPosition_ = pos; }
    
    // 사망 시 호출
    bool MarkDead(uint64 nowMs)
    {
        deadSinceMs_ = nowMs;
        
        if (not IsEnabled()) {
            state_ = RespawnState::Dead;
            return false;
        }
        
        state_ = RespawnState::Scheduled;
        ++respawnToken_;        // 예약 식별용
        return true;
    }
    
    void CancelScheduled()
    {
        if (state_ == RespawnState::Scheduled) {
            state_ = RespawnState::Dead;
            ++respawnToken_;    // 이전 예약 무효화
        }
    }
    
    bool CanExecuteRespawn(uint64 token) const
    {
        return IsEnabled() && IsScheduled() && respawnToken_ == token;
    }
    
    bool ExecuteRespawn(ObjectManager& om, IRespawnOwner& owner)
    {
        if (not IsEnabled()) return false;
        if (not IsScheduled()) return false;
        
        state_ = RespawnState::Respawning;
        
        owner.OnPreRespawn(om);
        owner.ApplyRespawnToWorld(om, respawnPosition_);
        owner.OnPostRespawn(om);
        
        if (policy_.invulMs > 0) {
            owner.GrantSpawnInvulnerability(om, policy_.invulMs);
        }
        
        state_ = RespawnState::Alive;
        return true;
    }
    
    uint32 GetDelayMs() const { return policy_.delayMs; }
    uint64 GetRespawnToken() const { return respawnToken_; }
    
private:
    RespawnPolicy policy_{};
    
    RespawnState state_{RespawnState::None};
    
    uint64 deadSinceMs_{0};
    uint64 respawnToken_{0};
    
    Vector3 respawnPosition_{0.0f, 0.0f, 0.0f};
    
};