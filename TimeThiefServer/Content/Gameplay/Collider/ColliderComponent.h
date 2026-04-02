#pragma once
#include "Physics/Collider/Collider.h"
#include "Content/Shared/BaseComponent.h"

/*---------------------
   ColliderComponent
---------------------*/
//
// ColliderComponent는 게임 오브젝트에 콜라이더 기능을 제공하는 컴포넌트입니다.
//

class ColliderComponent : public BaseComponent
{
public:
   void SetCollider(std::unique_ptr<SE::Physics::Collider> collider)
   {
      collider_ = std::move(collider);
   }
   
   SE::Physics::Collider* GetCollider() { return collider_.get(); }
   
private:
   std::unique_ptr<SE::Physics::Collider> collider_;
    
};
