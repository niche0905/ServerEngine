#pragma once
#include "ZoneTypes.h"

struct ZoneTable;
struct ZonePhaseData;
class Room;
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
    ZoneSystem() = default;
    
    bool Init(Room* ownerRoom, const ZoneBounds& bounds, const ZoneTable& zoneTable);
    void Update(float deltaTime);
    
    void Reset();
    
    bool IsInsideSafeZone(const SE::Math::Vector3& position) const;
    float GetDamagePerSecond() const;
    
    const ZoneCircle& GetCurrentZone() const { return currentZone_; }
    const ZoneCircle& GetStartZone() const { return startZone_; }
    const ZoneCircle& GetNextZone() const { return nextZone_; }
    uint32 GetCurrentPhase() const { return currentPhase_; }
    
private:
    void EnterNextPhase();
    void CalculateNextZone();
    void ApplyZoneDamage(float deltaTime);
    
    void BroadcastZoneChange();
    
private:
    Room*               ownerRoom_ = nullptr;
    
    ZoneBounds          zoneBounds_;
    const ZoneTable*    zoneTable_;
    
    uint32              currentPhase_ = 0;
 
    ZoneCircle          currentZone_{};
    ZoneCircle          startZone_{};
    ZoneCircle          nextZone_{};
    
    float               phaseElapsedTime_ = 0.0f;
    bool                isShrinking_ = false;
    
};
