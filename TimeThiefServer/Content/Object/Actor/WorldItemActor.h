#pragma once
#include "Actor.h"
#include "Content/Gameplay/Inventory/ItemTypes.h"

class ObjectManager;
class PlayerPawn;

/*-------------------
   WorldItemActor
-------------------*/
//
// WorldItemActor는 월드에 존재하는 아이템을 나타내는 액터입니다.
// 플레이어가 접근하여 아이템을 획득할 수 있습니다.
// 약간의 탄성을 가지고 떨어집니다.
//

class WorldItemActor : public Actor
{
public:
   WorldItemActor() = default;
   virtual ~WorldItemActor() = default;
   
   WorldItemActor(const WorldItemActor&) = delete;
   WorldItemActor& operator=(const WorldItemActor&) = delete;
   
public:
   const ItemStack& GetItemStack() const { return stack_; }
   void SetItemStack(const ItemStack& stack) { stack_ = stack; }
   
   void InitDrop(const ItemStack& stack, const Vector3& pos, const Vector3& initVel);
   
   void SetGravity(float gravity) { gravity_ = gravity; } 
   void SetRestitution(float restitution) { restitution_ = restitution; }
   void SetGroundFriction(float friction) { groundFriction_ = friction; }
   void SetLinearDamping(float damping) { linearDamping_ = damping; }
   
   bool IsSleeping() const { return sleeping_; }

protected:
   void OnSpawn() override;
   void Tick(float dt) override;
   
private:
   void IntegratePhysics(float dt);
   float QueryGroundHeight(ObjectManager& om, const Vector3& pos) const;
   
   void TrySleep(float dt;
   
private:
   ItemStack stack_{};
   
   Vector3 velocity_{};
   bool sleeping_{ false };
   
   float gravity_{ -9.81f };        // 중력 가속도 (m/s²) <- 수정이 필요할 수도 있다
   float restitution_{ 0.25 };      // 탄성 계수 (0~1 사이 값)
   float groundFriction_{ 0.8f };   // 지면 마찰 계수 (0~1 사이 값) x, z 축 속도에 적용
   float linearDamping_{ 0.05f };   // 공기 저항에 의한 감쇠 계수 (0~1 사이 값) 매 Tick마다
   
   float sleepVelSq_{ 0.05f * 0.05f };    // 속도 제곱이 이 값 이하이면 정지 상태로 간주 (m/s)²
   float sleepTimeAcc_{ 0.0f };           // 정지 상태 누적 시간 (초)
   float sleepTimeReq_{ 0.25f };          // 일정 시간 이상 정지 상태이면 완전 정지 처리 (초)
   
};
