#pragma once
#include <vector>
#include <unordered_set>
#include "Content/Object/ObjectId.h"
#include "Content/Gameplay/Replication/ReplicationEvent.h"
#include "Content/Object/ObjectEnum.h"

class IObjectReplicator;
class ProjectileActor;
struct RepFrame;
struct RepEvent;
class BaseObject;
class Room;

/*---------------------
   ReplicationSystem
---------------------*/
//
// ReplicationSystem는 RoomGameSystem에 포함되어 Room 내의 오브젝트에 대한 복제(Replication)와 관련된 기능을 담당하는 시스템입니다.
//

class ReplicationSystem
{
public:
   ReplicationSystem() = default;
   
   bool Init(Room* ownerRoom);

   void NotifySpawn(BaseObject* object);
   void NotifyDespawn(ObjectId objectId);
   
   void PushEvent(const RepEvent& event);
   
   void MarkDirty(ObjectId objectId);
   
   void FlushImmediate(const RepFrame& frame);
   void FlushPeriodic(const RepFrame& frame);
   
public:
   bool FlushProjectilePeriodic(ProjectileActor* projectile, ReplicationDirty flags, const RepFrame& frame, uint64 nowTimeMs);
   
private:
   const IObjectReplicator* GetReplicator(ObjectType objectType);
   
private:
   Room* ownerRoom_ = nullptr;   // non-owning
   
   std::vector<ObjectId> dirtyObjects_;   // 복제 대상이 된 오브젝트들의 ID 리스트
   std::unordered_set<ObjectId> dirtyObjectSet_;
   std::vector<RepEvent> replicationEvents_;
   
};
