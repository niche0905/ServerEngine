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
    
    bool Init(Room* ownerRoom, const ZoneBounds& bounds, const ZoneTable& zoneTable, float damageTickInterval);
    
    bool Start();
    void Update(float deltaTime);
    
    void ReStart();
    void Reset();
    
    void SetProgressing(bool isProgressing) { isProgressing_ = isProgressing; }
    void SetDamageApplied(bool isDamageApplied) { isDamageApplied_ = isDamageApplied; }
    
    bool IsInsideSafeZone(const SE::Math::Vector3& position) const;
    float GetDamagePerSecond() const;
    
    const ZoneCircle& GetCurrentZone() const { return currentZone_; }
    const ZoneCircle& GetStartZone() const { return startZone_; }
    const ZoneCircle& GetNextZone() const { return nextZone_; }
    uint32 GetCurrentPhase() const { return currentPhase_; }
    
private:
    // 축소 되는 과정
    void ProgressingZone(float deltaTime);
    
    // Zone에 의한 피해 적용
    void ZoneDamage(float deltaTime);
    
private:
    void EnterNextPhase();
    void CalculateNextZone();
    void ApplyZoneDamage(float tickInterval);
    
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
    
    float               damageTickElapsed_ = 0.0f;
    float               damageTickInterval_ = 1.0f;
    
    bool                isShrinking_ = false;
    
// Testing
private:
    bool                isProgressing_ = true;
    bool                isDamageApplied_ = true;
    
};
