#include "pch.h"
#include "StaticActor.h"

/*---------------
   StaticActor
---------------*/

void StaticActor::OnSpawn()
{
   Actor::OnSpawn();
}

void StaticActor::Tick(float dt)
{
   (void)dt;
   // StaticActor는 Tick 시 별도의 동작이 없습니다.
   // 필요 시 파생 클래스에서 재정의할 수 있습니다.
}
