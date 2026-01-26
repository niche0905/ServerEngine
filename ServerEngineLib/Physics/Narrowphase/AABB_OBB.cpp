#include "pch.h"
#include "Physics/Narrowphase/IntersectFns.h"
#include "Physics/Narrowphase/IntersectUtil.h"
#include "Physics/Collider/CollisionResult.h"
#include "Physics/Collider/AABBCollider.h"
#include "Physics/Collider/OBBCollider.h"

namespace SE::Physics::Narrowphase
{
   using Vector3 = SE::Math::Vector3;
   
   static inline float AbsF(float v) { return SE::Math::Abs(v); }
   
   bool Intersect_AABB_OBB(const Collider& a, const Collider& b, CollisionResult& out)
   {
      const auto& A = static_cast<const AABBCollider&>(a);
      const auto& B = static_cast<const OBBCollider&>(b);
      
      const Vector3 aMin = A.GetMin();
      const Vector3 aMax = A.GetMax();
      const Vector3 aC = (aMin + aMax) * 0.5f;
      const Vector3 aE = (aMax - aMin) * 0.5f;  // half extents
      
      const Vector3 Aaxis[3] = {
         Vector3{1, 0, 0},
         Vector3{0, 1, 0},
         Vector3{0, 0, 1}
      };
      
      const Vector3 bC = B.GetCenter();
      const Vector3 bE = B.GetHalfExtent();
      const Vector3 Baxis[3] = {
         B.GetAxisX(),
         B.GetAxisY(),
         B.GetAxisZ()
      };
      
      float R[3][3];
      float AbsR[3][3];
      
      const float eps = 1e-6f;
      for (int i = 0; i < 3; ++i) {
         for (int j = 0; j < 3; ++j) {
            R[i][j] = Aaxis[i].Dot(Baxis[j]);
            AbsR[i][j] = AbsF(R[i][j]) + eps;
         }
      }
      
      const Vector3 tW = bC - aC;
      float tA[3] = {
         tW.Dot(Aaxis[0]),
         tW.Dot(Aaxis[1]),
         tW.Dot(Aaxis[2])
      };
      
      const Vector3 d = aC - bC;
      
      float bestPen = 1e30f;
      Vector3 bestN{0, 0, 0};
      
      auto updateBest = [&](float pen, const Vector3& axisW)
      {
         if (pen < bestPen) {
            bestPen = pen;
            const float s = (d.Dot(axisW) >= 0.0f) ? 1.0f : -1.0f;
            bestN = axisW * s;
         }
      };
      
      auto testAxis = [&](float dist, float ra, float rb, const Vector3& axisW) -> bool
      {
         const float pen = (ra + rb) - AbsF(dist);
         if (pen < 0.0f) return false;
         updateBest(pen, axisW);
         return true;
      };
      
      /// A's axes
      // A0
      {
         const float ra = aE.x;
         const float rb = bE.x * AbsR[0][0] + bE.y * AbsR[0][1] + bE.z * AbsR[0][2];
         if (not testAxis(tA[0], ra, rb, Aaxis[0])) return false;
      }
      // A1
      {
         const float ra = aE.y;
         const float rb = bE.x * AbsR[1][0] + bE.y * AbsR[1][1] + bE.z * AbsR[1][2];
         if (not testAxis(tA[1], ra, rb, Aaxis[1])) return false;
      }
      // A2
      {
         const float ra = aE.z;
         const float rb = bE.x * AbsR[2][0] + bE.y * AbsR[2][1] + bE.z * AbsR[2][2];
         if (not testAxis(tA[2], ra, rb, Aaxis[2])) return false;
      }
      
      /// B's axes
      for (int j = 0; j < 3; ++j) {
         const float dist = tW.Dot(Baxis[j]);
         const float ra = aE.x * AbsR[0][j] + aE.y * AbsR[1][j] + aE.z * AbsR[2][j];
         const float rb = ((j == 0) ? bE.x : (j == 1 ? bE.y : bE.z));
         if (not testAxis(dist, ra, rb, Baxis[j])) return false;
      }
      
      auto axisFromCross = [&](int i, int j) -> Vector3
      {
         return Aaxis[i].Cross(Baxis[j]);
      };
      
      auto getAE = [&](int idx) -> float
      {
         return ((idx == 0) ? aE.x : (idx == 1 ? aE.y : aE.z));
      };
      auto getBE = [&](int idx) -> float
      {
         return ((idx == 0) ? bE.x : (idx == 1 ? bE.y : bE.z));
      };
      
      for (int i = 0; i < 3; ++i) {
         for (int j = 0; j < 3; ++j) {
            
            Vector3 axis = axisFromCross(i, j);
            const float axisLenSq = axis.LengthSq();
            if (axisLenSq <= 1e-12f)
               continue; // 거의 영벡터인 경우 스킵 (평행한 축)
            
            // axis = axis * (1.0f / std::sqrt(axisLenSq)); // 정규화
            
            const int i1 = (i + 1) % 3;
            const int i2 = (i + 2) % 3;
            const int j1 = (j + 1) % 3;
            const int j2 = (j + 2) % 3;
            
            const float ra = getAE(i1) * AbsR[i2][j] + getAE(i2) * AbsR[i1][j];
            const float rb = getBE(j1) * AbsR[i][j2] + getBE(j2) * AbsR[i][j1];
            
            const float dist = tW.Dot(axis);
            if (not testAxis(dist, ra, rb, axis)) return false;
         }
      }
      
      out.hit = true;
      out.normal = bestN;
      out.penetration = bestPen;
      
      // point는 AABB의 중심에서 충돌 법선 방향으로 반발한 위치로 설정 (문제가 될 시 추후 개선)
      out.point = aC - bestN * (bestPen * 0.5f);
      
      return true;
   }

   bool Intersect_OBB_AABB(const Collider& a, const Collider& b, CollisionResult& out)
   {
      return SwapWrapper(a, b, out, &Intersect_AABB_OBB);
   }
}
