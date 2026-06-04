#pragma once
#include <vector>
#include <unordered_set>
#include "Content/Object/ObjectId.h"
#include "Content/Gameplay/Replication/ReplicationEvent.h"
#include "Content/Gameplay/Replication/Replicator/MonsterReplicator.h"
#include "Content/Gameplay/Replication/Replicator/PlayerReplicator.h"
#include "Content/Gameplay/Replication/Replicator/ProjectileReplicator.h"
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

   void PushEvent(const RepEvent& event);
   
   void MarkDirty(ObjectId objectId);
   
   void FlushImmediate(const RepFrame& frame);
   void FlushPeriodic(const RepFrame& frame);
   
private:
   const IObjectReplicator* GetReplicator(ObjectType objectType);
   
   void NormalizeEvent(RepEvent& ev, const RepFrame& frame, uint64 nowMs) const;
   void DispatchImmediateEvent(const RepEvent& ev, const RepFrame& frame) const;

private:
   void FlushEvent_Spawn(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Despawn(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Death(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Respawn(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Health(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_MaxHealth(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Money(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Explosion(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_PickupItem(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Item(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_EquipItem(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_UseItem(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_ChestInteract(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Jump(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_JumpLand(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_DoubleJump(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Crouch(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_WireLaunch(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_WireAction(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_WireActionEnd(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Aim(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Fire(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Reload(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_WeaponChanged(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Hit(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_GrenadeThrow(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_GrenadeMoveSync(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_GrenadeExplosion(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_Attack(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_MonsterFire(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_MonsterImpact(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_WeaponStatChange(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_KillPlayer(const RepEvent& ev, const RepFrame& frame) const;
   void FlushEvent_ZoneChange(const RepEvent& ev, const RepFrame& frame) const;
   
   void FlushEvent_ZoneFlow(const RepEvent& ev, const RepFrame& frame) const;
   
private:
   Room*                         ownerRoom_ = nullptr;   // non-owning
   
   std::vector<ObjectId>         dirtyObjects_;   // 복제 대상이 된 오브젝트들의 ID 리스트
   std::unordered_set<ObjectId>  dirtyObjectSet_;
   std::vector<RepEvent>         replicationEvents_;
   
private:
   PlayerReplicator              playerReplicator_;
   MonsterReplicator             monsterReplicator_;
   ProjectileReplicator          projectileReplicator_;
   
};
