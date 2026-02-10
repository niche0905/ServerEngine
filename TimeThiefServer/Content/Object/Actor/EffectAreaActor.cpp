#include "pch.h"
#include "EffectAreaActor.h"

/*-------------------
   EffectAreaActor
-------------------*/

void EffectAreaActor::Init(ObjectId ownerId, const Vector3& pos, uint32 lifetimeMs, uint32 periodMs)
{
    ownerId_ = ownerId;
    lifetimeMs_ = lifetimeMs;
    periodMs_ = periodMs;
    
    SetPosition(pos);
}

void EffectAreaActor::OnSpawn()
{
    Actor::OnSpawn();
    
    accSec_ = 0.0f;
    
    // TODO: lifetime 타이머 시작
    // RegisterLifetimeMs(nowMs, lifetimeMs_);
}

void EffectAreaActor::Tick(float dt)
{
    Actor::Tick(dt);
    
    if (periodMs_ == 0)
        return;
    
    const float periodSec = static_cast<float>(periodMs_) / 1000.0f;
    if (periodSec < 0.0f)
        return;
    
    int32 safeLoop = 0;
    while (accSec_ >= periodSec and safeLoop++ < 4) {
        accSec_ -= periodSec;
        
        // TODO: Room이 EffectActor::Pulse를 호출하도록
        // Pulse는 om을 필요로 하므로, Room에서 호출 시 om을 전달해야 함
    }
}

void EffectAreaActor::Pulse(ObjectManager& om, uint32 nowMs)
{
    std::vector<ObjectId> targets;
    GatherTargets(om, targets);
    
    for (const auto& targetId : targets) {
        if (not CanEffectTarget(om, targetId)) continue;
        
        ApplyToTarget(om, targetId, nowMs);
    }
}

bool EffectAreaActor::CanEffectTarget(ObjectManager& om, ObjectId targetId) const
{
    (void)om;
    
    // 기본적으로 소유자 자신은 효과를 받지 않음
    if (targetId == ownerId_) return false;
    
    // TODO: 추가 조건 검사 (예: 팀 체크, 상태 이상 등)
    
    return true;
}

void EffectAreaActor::GatherTargets(ObjectManager& om, std::vector<ObjectId>& outTargets) const
{
    (void)om;
    
    // TODO: Collider 시스템 연동 필요
    // 현재는 빈 목록 반환
    
    outTargets.clear();
}
