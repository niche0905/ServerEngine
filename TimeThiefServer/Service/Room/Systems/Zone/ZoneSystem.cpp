#include "pch.h"
#include "ZoneSystem.h"

#include <random>

#include "Data/Tables/ZoneTable.h"

namespace 
{
   float Clamp01(float value)
   {
      return std::clamp(value, 0.0f, 1.0f);
   }
   
   float Lerp(float a, float b, float t)
   {
      return a + (b - a) * t;
   }
   
   SE::Math::Vector3 Lerp(const SE::Math::Vector3& a, const SE::Math::Vector3& b, float t)
   {
      return SE::Math::Vector3{
         Lerp(a.x, b.x, t),
         Lerp(a.y, b.y, t),
         Lerp(a.z, b.z, t)
      };
   }
   
   SE::Math::Vector3 RandPointInCircle(float radius)
   {
      SE::Math::Vector3 point{};
      
      do
      {
         // TODO: rand 말고 std::mt19937과 std::uniform_real_distribution을 사용하여 더 좋은 난수 생성 방식으로 변경하기
         point.x = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;   // -1.0 ~ 1.0
         point.y = (rand() / (float)RAND_MAX) * 2.0f - 1.0f;   // -1.0 ~ 1.0
      }
      while (point.LengthSq() > 1.0f);   // 단위 원 안에 있는 점이 나올 때까지 반복
      
      return point * radius;
   }
}

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
   
   Reset();
   
   return true;
}

void ZoneSystem::Update(float deltaTime)
{
   if (ownerRoom_ == nullptr or zoneTable_ == nullptr)
      return;
   
   if (deltaTime <= 0.0f)
      return;
   
   phaseElapsedTime_ += deltaTime;
   
   const ZonePhaseData& phaseData = zoneTable_->GetPhaseData(currentPhase_);
   
   const float waitDuration = phaseData.waitTimeSeconds;
   const float shrinkDuration = phaseData.shrinkTimeSeconds;
   
   if (isShrinking_ == false) {
      // 대기 단계
      if (phaseElapsedTime_ >= waitDuration) {
         phaseElapsedTime_ -= waitDuration;
         isShrinking_ = true;
      }
   }
   else{
      // 수축 단계
      float t = Clamp01(phaseElapsedTime_ / shrinkDuration);
      
      currentZone_.center = Lerp(startZone_.center, nextZone_.center, t);
      currentZone_.radius = Lerp(startZone_.radius, nextZone_.radius, t);
      
      if (phaseElapsedTime_ >= shrinkDuration) {
         currentZone_ = nextZone_;
         EnterNextPhase();
      }
   }
   
   ApplyZoneDamage(deltaTime);
}

void ZoneSystem::Reset()
{
   currentPhase_ = 0;
   phaseElapsedTime_ = 0.0f;
   isShrinking_ = false;
   
   startZone_ = ZoneCircle{ zoneBounds_.center, 1000000.0f };   // 초기 Zone은 매우 큰 반지름으로 설정하여 사실상 모든 영역이 안전지대가 되도록 함
   currentZone_ = startZone_;
   CalculateNextZone();
}

bool ZoneSystem::IsInsideSafeZone(const SE::Math::Vector3& position) const
{
   return currentZone_.Contains(position);
}

float ZoneSystem::GetDamagePerSecond() const
{
   if (zoneTable_ == nullptr)
      return 0.0f;
   
   const ZonePhaseData& phaseData = zoneTable_->GetPhaseData(currentPhase_);
   return phaseData.damagePerSecond;
}

void ZoneSystem::EnterNextPhase()
{
   phaseElapsedTime_ = 0.0f;
   isShrinking_ = false;
   
   ++currentPhase_;
   if (currentPhase_ >= zoneTable_->Phase()) {
      // 더 이상 진행할 Phase가 없다
      return;
   }
   
   const ZonePhaseData& phaseData = zoneTable_->GetPhaseData(currentPhase_);
   startZone_ = currentZone_;
   CalculateNextZone();
   
   // TODO: Phase가 변경되었음을 Room에 알리는 로직 추가 (예: 패킷 전송 등) Broadcast 할 것
}

void ZoneSystem::CalculateNextZone()
{
   if (zoneTable_ == nullptr)
      return;
   
   const ZonePhaseData& phaseData = zoneTable_->GetPhaseData(currentPhase_);
   
   const float nextRadius = phaseData.radius;
   const float movableRange = std::max(0.0f, startZone_.radius - nextRadius);
   
   SE::Math::Vector3 newCenter = startZone_.center;
   
   if (movableRange > 0.001f) {
      newCenter = newCenter + RandPointInCircle(movableRange);
   }
   
   {
      const auto minPos = zoneBounds_.GetMin();
      const auto maxPos = zoneBounds_.GetMax();
      
      newCenter.x = std::clamp(newCenter.x, minPos.x + nextRadius, maxPos.x - nextRadius);
      newCenter.y = std::clamp(newCenter.y, minPos.y + nextRadius, maxPos.y - nextRadius);
      newCenter.z = 0.0f;   // 수평 평면에서만 움직이도록 z 좌표는 0으로 고정
   }
   
   nextZone_.center = newCenter;
   nextZone_.radius = nextRadius;
}

void ZoneSystem::ApplyZoneDamage(float deltaTime)
{
   if (ownerRoom_ == nullptr)
      return;
   
   const float damagePerSecond = GetDamagePerSecond();
   if (damagePerSecond <= 0.0f)
      return;
   
   const float damage = damagePerSecond * deltaTime;
   if (damage <= 0.0f)
      return;
   
   // TODO: Room이 제공하는 Pawn 순회 및 자기장 데미지 적용
   
}
