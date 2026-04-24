#include "pch.h"
#include "ProjectileReplicator.h"
#include "Content/Object/Actor/ProjectileActor.h"
#include "Service/Room/Room.h"


ReplicateResult ProjectileReplicator::FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame,
                                                    uint64 nowMs, Room& room) const
{
    auto* projectile = dynamic_cast<ProjectileActor*>(obj);
    if (!projectile)
        return ReplicateResult{};
    
    return FlushProjectilePeriodic(*projectile, flags, frame, nowMs, room);
}

ReplicateResult ProjectileReplicator::FlushProjectilePeriodic(ProjectileActor& projectile, ReplicationDirty flags,
    const RepFrame& frame, uint64 nowMs, Room& room) const
{
    ReplicateResult result;
    
    if (HasDirty(flags, ReplicationDirty::Transform)) {
        
        if (nowMs >= projectile.GetReplicatedState().lastReplicatedTimeMs + 100) {   // TODO: 100ms이거 상수나 config 값으로 빼기
            
            se::game::N_Move projectileMoveNoti;
            {
                auto* entityIdPtr = projectileMoveNoti.mutable_entity_id();
                entityIdPtr->set_value(projectile.GetId().value);
      
                projectileMoveNoti.set_object_type(se::common::OBJ_PROJECTILE);
      
                auto* transformPtr = projectileMoveNoti.mutable_transform();
                auto* positionPtr = transformPtr->mutable_position();
                const auto& pos = projectile.GetPosition();
                positionPtr->set_x(pos.x);
                positionPtr->set_y(pos.y);
                positionPtr->set_z(pos.z);
                transformPtr->set_yaw(projectile.GetYaw());    // 의미 없는 정보이긴 함 (velocity 방향으로 날아가게 할 것이기에) 
      
                auto* projectileMovementPtr = projectileMoveNoti.mutable_projectile_movement();
                auto* directionPtr = projectileMovementPtr->mutable_velocity();
                const auto& velocity = projectile.GetVelocity();
                directionPtr->set_x(velocity.x);
                directionPtr->set_y(velocity.y);
                directionPtr->set_z(velocity.z);
            }
            
            SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(projectileMoveNoti);
            // 모두에게 Broadcast
            room.BroadcastReplication(sendBuffer);
            
            result.sent = true;
            result.handled |= ReplicationDirty::Transform;
        }
    }
    
    return result;
}
