#pragma once
#include "Actor.h"

/*---------------
   StaticActor
---------------*/
//
// StaticActor는 고정된 World Object의 기본 클래스입니다
// 기본적으로 움직이지 않고,
// 상호작용이 가능한 상자/문/레버 등의 오브젝트를 구현할 때 사용됩니다.
//

class StaticActor : public Actor
{
public:
   StaticActor() = default;
   virtual ~StaticActor() = default;
   
   StaticActor(const StaticActor&) = delete;
   StaticActor& operator=(const StaticActor&) = delete;
   
public:
   bool IsStatic() const { return true; }

protected:
   void SetPositionStatic(const Vector3& position) { Actor::SetPosition(position); }
   
protected:
   virtual void OnSpawn() override;
   virtual void Tick(float dt) override;
    
};
