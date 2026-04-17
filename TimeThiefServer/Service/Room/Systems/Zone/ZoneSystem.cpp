#include "pch.h"
#include "ZoneSystem.h"
#include <random>

#include "Content/Object/Actor/Pawn.h"
#include "Data/Tables/ZoneTable.h"
#include "Service/Room/Room.h"

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
      // TODO: std::mt19937이 아닌 재현 가능한 Rng를 만들어서 사용하자
      static thread_local std::mt19937 rng(std::random_device{}());
      static thread_local std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

      float r = std::sqrt(dist01(rng)) * radius;
      float theta = dist01(rng) * 2.0f * Pi;

      return SE::Math::Vector3{
         r * std::cos(theta),
         r * std::sin(theta),
         0.0f
     };
   }
}

/*--------------
   ZoneSystem
--------------*/

bool ZoneSystem::Init(Room* ownerRoom, const ZoneBounds& bounds, const ZoneTable& zoneTable, float damageTickInterval)
{
   if (ownerRoom == nullptr)
      return false;
   
   ownerRoom_ = ownerRoom;
   zoneBounds_ = bounds;
   zoneTable_ = &zoneTable;
   damageTickInterval_ = damageTickInterval;
   
   return true;
}

bool ZoneSystem::Start()
{
   Reset();
   CalculateNextZone();
   
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
   
   damageTickElapsed_ += deltaTime;
   while (damageTickElapsed_ >= damageTickInterval_) {
      damageTickElapsed_ -= damageTickInterval_;
      ApplyZoneDamage(damageTickInterval_);
   }
}

void ZoneSystem::Reset()
{
   currentPhase_ = 0;
   phaseElapsedTime_ = 0.0f;
   isShrinking_ = false;
   damageTickElapsed_ = 0.0f;
   
   startZone_ = ZoneCircle{ zoneBounds_.center, 1000000.0f };   // 초기 Zone은 매우 큰 반지름으로 설정하여 사실상 모든 영역이 안전지대가 되도록 함
   currentZone_ = startZone_;
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
   
   BroadcastZoneChange();
}

void ZoneSystem::ApplyZoneDamage(float tickInterval)
{
   if (ownerRoom_ == nullptr)
      return;
   
   const float damagePerSecond = GetDamagePerSecond();
   if (damagePerSecond <= 0.0f)
      return;
   
   const float damage = damagePerSecond * tickInterval;
   if (damage <= 0.0f)
      return;
   
   ObjectManager& objectManager = ownerRoom_->GetObjectManager();
   
   // 안정성 괜찮나..? float라 좀 걱정됨
   int32 damageInt = static_cast<int32>(std::floor(damage));
   ownerRoom_->ForEachPawn([this, &objectManager, damageInt](Pawn& pawn)
   {
      if (!pawn.IsHpAlive()) // 이미 죽은 Pawn은 피해를 입힐 필요가 없음
         return;
      
      if (IsInsideSafeZone(pawn.GetPosition())) // 안전지대 안에 있는 Pawn은 피해를 입힐 필요가 없음
         return;
      
      pawn.ApplyDamage(objectManager, damageInt, DamageContext{0, 0, 0, DamageType::Zone, DamageSource::Environment});
   });
}

void ZoneSystem::BroadcastZoneChange()
{
   if (ownerRoom_ == nullptr)
      return;
   
   ownerRoom_->OnZoneChanged(currentPhase_, nextZone_, zoneTable_->GetPhaseData(currentPhase_).waitTimeSeconds, zoneTable_->GetPhaseData(currentPhase_).shrinkTimeSeconds);
}
