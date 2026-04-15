#pragma once
#include "Content/Object/Actor/ProjectileActor.h"

/*---------------
   RocketActor
---------------*/
//
// RocketActor는 로켓 발사체를 나타내는 클래스입니다.
// 직선 운동을 하며 충돌 시 폭발 효과를 발생시키는 것을 예상하여 설계되었습니다.
//

class RocketActor : public ProjectileActor
{
public:
   RocketActor() = default;
   virtual ~RocketActor() = default;
   
   RocketActor(const RocketActor&) = delete;
   RocketActor& operator=(const RocketActor&) = delete;
   
public:
   // virtual void OnSpawn() override;
   // virtual void Tick(float dt) override;
   
public:
   virtual void OnHit(ObjectManager& om, ObjectId hitObjectId) override;
    
};
