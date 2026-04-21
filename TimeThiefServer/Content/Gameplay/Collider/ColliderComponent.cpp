#include "pch.h"
#include "ColliderComponent.h"

#include "Content/Object/Actor/Actor.h"
#include "Physics/Ray/RaycastHit.h"

/*---------------------
   ColliderComponent
---------------------*/

void ColliderComponent::Init(BaseObject* owner, ColliderRole role,
    std::unique_ptr<SE::Physics::Collider> collider)
{
    SetOwner(owner);
    SetOwnerActor(static_cast<Actor*>(owner));
    SetRole(role);
    SetCollider(std::move(collider));
}

void ColliderComponent::SetCollider(std::unique_ptr<SE::Physics::Collider> collider)
{
    collider_ = std::move(collider);
}

SE::Physics::Collider* ColliderComponent::GetCollider()
{
    return collider_.get(); 
}

const SE::Physics::Collider* ColliderComponent::GetCollider() const
{
    return collider_.get(); 
}

void ColliderComponent::UpdateWorldCollider()
{
    if (ownerActor_ == nullptr, collider_ == nullptr)
        return;
    
    collider_->UpdateWorld(ownerActor_->GetPosition(), ownerActor_->GetYaw());
}
