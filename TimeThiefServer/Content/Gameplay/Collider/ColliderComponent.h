#pragma once
#include "Physics/Collider/Collider.h"
#include "Content/Shared/BaseComponent.h"

class Actor;

enum class ColliderRole
{
   Movement,
   Hit,
   Hurtbox,
   Hitbox,
   Trigger
};

/*---------------------
   ColliderComponent
---------------------*/
//
// ColliderComponent는 게임 오브젝트에 콜라이더 기능을 제공하는 컴포넌트입니다.
//

class ColliderComponent : public BaseComponent
{
public:
   void SetOwner(Actor* owner) { owner_ = owner; }
   Actor* GetOwner() const { return owner_; }
   
   void SetCollider(std::unique_ptr<SE::Physics::Collider> collider)
   {
      collider_ = std::move(collider);
   }
   
   SE::Physics::Collider* GetCollider() { return collider_.get(); }
   
   void SetRole(ColliderRole role) { role_ = role; }
   ColliderRole GetRole() const { return role_; }
   
private:
   Actor* owner_ = nullptr;   // ColliderComponent가 부착된 Actor에 대한 포인터 (필요하다면 사용)
   ColliderRole role_ = ColliderRole::Trigger;
   std::unique_ptr<SE::Physics::Collider> collider_;
    
};
