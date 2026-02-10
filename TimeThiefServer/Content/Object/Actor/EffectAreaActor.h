#pragma once
#include "Actor.h"

/*-------------------
   EffectAreaActor
-------------------*/
//
// EffectAreaActor는 특정 지역에 효과를 표현하는 액터입니다.
// 예를 들어, 독구름, 치유의 영역, 마법진 등이 이에 해당합니다.
// lifetime이 끝나면 자동으로 제거됩니다.
//

class EffectAreaActor : public Actor
{
public:
   EffectAreaActor() = default;
   virtual ~EffectAreaActor() = default;
   
   EffectAreaActor(const EffectAreaActor&) = delete;
   EffectAreaActor& operator=(const EffectAreaActor&) = delete;
   
public:
   ObjectId GetOwner() const { return ownerId_; }
   void SetOwner(ObjectId ownerId) { ownerId_ = ownerId; }
   
public:
   void Init(ObjectId ownerId, const Vector3& pos, uint32 lifetimeMs, uint32 periodMs);
   
protected:
   void OnSpawn() override;
   void Tick(float dt) override;
   
protected:
   void Pulse(ObjectManager& om, uint32 nowMs);
   
   virtual void ApplyToTarget(ObjectManager& om, ObjectId targetId, uint64 nowMs) = 0;
   
   virtual bool CanEffectTarget(ObjectManager& om, ObjectId targetId) const;
   
   virtual void GatherTargets(ObjectManager& om, std::vector<ObjectId>& outTargets) const;
   
private:
   ObjectId ownerId_{};
   
   uint32 lifetimeMs_{ 0 };
   uint32 periodMs_{ 0 };  // 0이면 주기 없음
   
   float accSec_{ 0.0f };
    
};
