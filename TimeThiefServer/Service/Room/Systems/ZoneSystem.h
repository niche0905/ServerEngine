#pragma once
#include "ZoneTypes.h"

class MapData;
struct ZoneCircle;

/*--------------
   ZoneSystem
--------------*/
//
// ZoneSystem은 Room 내에서 TimeStorm을 관리하는 시스템입니다.
//

class ZoneSystem
{
public:
    void Init(const MapData& map_data);
    // TODO: Tick 기반 말고 Timer 기반 고려..
    void Update(float deltaTime);
    
    bool IsInsideSafeZone(const SE::Math::Vector3& position) const;
    float GetDamagePerSecond() const;
    
    void NextZoneCalculate();
    
private:
    uint32 currentPhase_ = 0;
    ZoneCircle currentZone_{};
    ZoneCircle nextZone_{};
    float shrinkTimer_ = 0.0f;
    
};
