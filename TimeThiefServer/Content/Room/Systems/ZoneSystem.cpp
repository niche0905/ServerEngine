#include "pch.h"
#include "ZoneSystem.h"

/*--------------
   ZoneSystem
--------------*/

void ZoneSystem::Init(const MapData& map_data)
{
   // TODO: 맵 데이터에서 초기 Zone 정보 설정
   //       굳이 맵 데이터가 아니더라도 초기 Zone 정보를 설정할 수 있는 방법이 필요
}

void ZoneSystem::Update(float deltaTime)
{
   // TODO: Zone의 shrinkTimer_를 감소시키고, shrinkTimer_가 0이 되면 currentZone_을 nextZone_으로 업데이트하고, nextZone_을 새롭게 계산하는 로직 구현
   //       또한, Zone이 shrink되는 동안 플레이어가 Zone 밖에 있다면 피해를 입히는 로직도 구현 필요
   //       ZoneTable이 먼저 작성되어야 한다
}

bool ZoneSystem::IsInsideSafeZone(const SE::Math::Vector3& position) const
{
   return currentZone_.Contains(position);
}

float ZoneSystem::GetDamagePerSecond() const
{
   // TODO: ZoneTable을 참조하여 현재 Phase에 따른 초당 피해량 반환
   return 0.0f;
}

void ZoneSystem::NextZoneCalculate()
{
   // TODO: CurrnetZone의 내부에 NextZone이 생성되도록 구현
   //       ZoneTable이 먼저 작성되어야 한다 (Phase에 따른 반지름을 참조해야 하기 때문)
}
