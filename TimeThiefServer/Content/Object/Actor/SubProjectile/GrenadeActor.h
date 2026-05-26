#pragma once
#include "Content/Object/Actor/ProjectileActor.h"

/*----------------
   GrenadeActor
----------------*/
//
// GrenadeActor는 수류탄 발사체를 나타내는 클래스입니다.
// Client Ownership하게 움직이기에 서버에서는 위치 동기화와 폭발 처리에 집중하는 형태로 설계되었습니다.
//

class GrenadeActor : public ProjectileActor
{
public:
   GrenadeActor() = default;
   virtual ~GrenadeActor() = default;
   
   GrenadeActor(const GrenadeActor&) = delete;
   GrenadeActor& operator=(const GrenadeActor&) = delete;
   
public:
   void Explode(ObjectManager& om);
   
protected:
   virtual void OnExplode(ObjectManager& om) override;
    
};
