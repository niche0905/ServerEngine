#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Narrowphase/IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/OBBCollider.h"

namespace SE::Physics::NarrowPhase
{
   using Vector3 = SE::Math::Vector3;
   
   static inline float AbsF(float v) { return SE::Math::Abs(v); }

   bool Intersect_OBB_OBB(const Collider& a, const Collider& b, CollisionResult& out)
   {
      const auto& A = static_cast<const OBBCollider&>(a);
      const auto& B = static_cast<const OBBCollider&>(b);
      
      const Vector3 aC = A.GetCenter();
      const Vector3 aE = A.GetHalfExtent();
      const Vector3 aAxis[3] = { A.GetAxisX(), A.GetAxisY(), A.GetAxisZ() };
      
      const Vector3 bC = B.GetCenter();
      const Vector3 bE = B.GetHalfExtent();
      const Vector3 bAxis[3] = { B.GetAxisX(), B.GetAxisY(), B.GetAxisZ() };
      
      const Vector3 tW = bC - aC;
      
      float R[3][3];
      float AbsR[3][3];
      constexpr float eps = 1e-6f;
      
      for (int i = 0; i < 3; ++i) {
         for (int j = 0; j < 3; ++j) {
            R[i][j] = aAxis[i].Dot(bAxis[j]);
            AbsR[i][j] = AbsF(R[i][j]) + eps;
         }
      }
      
      float tA[3] = {
         tW.Dot(aAxis[0]),
         tW.Dot(aAxis[1]),
         tW.Dot(aAxis[2])
      };
      
      float bestPen = 1e30f;
      Vector3 bestN{0, 0, 0};
      
      const Vector3 d = aC - bC;
      
      auto updateBest = [&](float pen, const Vector3& axisW)
      {
         if (pen < bestPen) {
            bestPen = pen;
            const float s = (d.Dot(axisW) >= 0) ? 1.0f : -1.0f;
            bestN = axisW * s;
         }
      };
      
      auto testAxis = [&](float dist, float ra, float rb, const Vector3& axisW) -> bool
      {
         float pen = (ra + rb) - AbsF(dist);
         if (pen < 0.0f) 
            return false;
         
         updateBest(pen, axisW);
         return true;
      };
      
      auto getAE = [&](int i) -> float { return (i == 0) ? aE.x : (i == 1) ? aE.y : aE.z; };
      auto getBE = [&](int j) -> float { return (j == 0) ? bE.x : (j == 1) ? bE.y : bE.z; };
      
      // A의 축들
      for (int i = 0; i < 3; ++i) {
         const float ra = getAE(i);
         const float rb = bE.x * AbsR[i][0] + bE.y * AbsR[i][1] + bE.z * AbsR[i][2];
         if (not testAxis(tA[i], ra, rb, aAxis[i]))
            return false;
      }
      
      // B의 축들
      for (int j = 0; j < 3; ++j) {
         const float dist = tW.Dot(bAxis[j]);
         const float ra = aE.x * AbsR[0][j] + aE.y * AbsR[1][j] + aE.z * AbsR[2][j];
         const float rb = getBE(j);
         if (not testAxis(dist, ra, rb, bAxis[j]))
            return false;
      }
      
      // 교차축들
      for (int i = 0; i < 3; ++i) {
         const int i1 = (i + 1) % 3;
         const int i2 = (i + 2) % 3;
         
         for (int j = 0; j < 3; ++j) {
            const int j1 = (j + 1) % 3;
            const int j2 = (j + 2) % 3;
            
            // 거의 평행이라면 건너뜀
            Vector3 axisW = aAxis[i].Cross(bAxis[j]);
            const float axisLenSq = axisW.LengthSq();
            if (axisLenSq < 1e-12f) continue;
            
            axisW = axisW * (1.0f / std::sqrt(axisLenSq));
            
            const float dist = tA[i2] * R[i1][j] - tA[i1] * R[i2][j];
            const float ra = getAE(i1) * AbsR[i2][j] + getAE(i2) * AbsR[i1][j];
            const float rb = getBE(j1) * AbsR[i][j2] + getBE(j2) * AbsR[i][j1];
            
            if (not testAxis(dist, ra, rb, axisW))
               return false;
         }
      }
      
      // 충돌 정보 기록
      out.hit = true;
      out.normal = bestN;
      out.penetration = bestPen;
      
      // 충돌 지점 계산 (간단히 A의 중심에서 노멀 방향으로 반발 거리만큼 이동한 지점)
      out.point = (aC + bC) * 0.5f - bestN * (bestPen * 0.5f);
      
      return true;
   }
    
}
