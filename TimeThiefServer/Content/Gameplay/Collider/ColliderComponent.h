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
   void Init(ObjectId owner, Actor* ownerActor, ColliderRole role, std::unique_ptr<SE::Physics::Collider> collider);
   
public:
   void SetOwnerActor(Actor* ownerActor) { ownerActor_ = ownerActor; }
   Actor* GetOwnerActor() const { return ownerActor_; }
   
   void SetCollider(std::unique_ptr<SE::Physics::Collider> collider);
   SE::Physics::Collider* GetCollider();
   const SE::Physics::Collider* GetCollider() const;
   
   bool HasCollider() const { return collider_ != nullptr; }
   
   void UpdateWorldCollider();
   
   void SetRole(ColliderRole role) { role_ = role; }
   ColliderRole GetRole() const { return role_; }
   
private:
   Actor* ownerActor_ = nullptr;   // ColliderComponent가 부착된 Actor (Owner)
   
   ColliderRole role_ = ColliderRole::Trigger;
   std::unique_ptr<SE::Physics::Collider> collider_;
    
};
