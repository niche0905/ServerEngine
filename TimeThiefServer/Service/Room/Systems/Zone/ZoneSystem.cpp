#include "pch.h"
#include "ZoneSystem.h"
#include "Data/Tables/ZoneTable.h"

/*--------------
   ZoneSystem
--------------*/

bool ZoneSystem::Init(Room* ownerRoom, const ZoneBounds& bounds, const ZoneTable& zoneTable)
{
   if (ownerRoom == nullptr)
      return false;
   
   ownerRoom_ = ownerRoom;
   zoneBounds_ = bounds;
   zoneTable_ = &zoneTable;
   
   return true;
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
