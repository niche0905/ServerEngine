#pragma once

class Pawn;

namespace SE::Physics
{
   namespace Hit
   {
      struct HitResult;
   }

   struct Ray;
}

struct ObjectId;
class ServerMap;
class Room;

/*----------------
   CombatSystem
----------------*/
//
// CombatSystem는 Room 내에서 전투 관련 로직과 상태를 관리하는 시스템입니다.
//

class CombatSystem
{
public:
   CombatSystem() = default;
   
   bool Init(Room* ownerRoom, const ServerMap& mapData);
   
   bool TraceHit(const SE::Physics::Ray& ray, ObjectId exceptId, SE::Physics::Hit::HitResult& outHit) const;
   bool LaunchRocket(const SE::Math::Vector3& pos, const SE::Math::Vector3& dir, Pawn* ownerPawn, int32 damage, float speed, uint32 lifetimeMs, float radius);
   
   void ProjectileExplosion(ObjectId projectileId, const SE::Math::Vector3& pos, ObjectId ownerId, int32 damage, float radius, bool distanceDamageEnabled);
  
private:
   bool IsExplosionBlocked(const SE::Physics::Ray& ray, float dist, ObjectId targetId) const;

private:
   Room*                            ownerRoom_ = nullptr;   // non-owning
   const ServerMap*                 mapData_ = nullptr;     // non-owning
    
};
