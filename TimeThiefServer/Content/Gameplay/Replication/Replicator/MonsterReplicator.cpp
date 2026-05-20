#include "pch.h"
#include "MonsterReplicator.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Service/Room/Room.h"


ReplicateResult MonsterReplicator::FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame,
    uint64 nowMs, Room& room) const
{
    auto* monster = dynamic_cast<MonsterPawn*>(obj);
    if (!monster)
        return ReplicateResult{};
    
    return FlushMonsterPeriodic(*monster, flags, frame, nowMs, room);
}

ReplicateResult MonsterReplicator::FlushMonsterPeriodic(MonsterPawn& monster, ReplicationDirty flags,
    const RepFrame& frame, uint64 nowMs, Room& room) const
{
    ReplicateResult result;
    
    // TODO: 시간 제한 필요할 듯? last update time
    
    constexpr uint64 MonsterReplicateIntervalMs = 100;   // TODO: 상수나 config 값으로 빼기
    
    const uint64 lastMs = monster.GetReplicatedState().lastReplicatedTimeMs;
    
    if (HasDirty(flags, ReplicationDirty::Transform)) {
        
        if ((nowMs - lastMs) >= MonsterReplicateIntervalMs)
        {
            se::game::N_Move noti;
            {
                auto* entityIdPtr = noti.mutable_entity_id();
                entityIdPtr->set_value(monster.GetId().value);
            
                noti.set_object_type(se::common::OBJ_MONSTER);
            
                auto* transformPtr = noti.mutable_transform();
                auto* positionPtr = transformPtr->mutable_position();
                const auto& newPos = monster.GetPosition();
                positionPtr->set_x(newPos.x);
                positionPtr->set_y(newPos.y);
                positionPtr->set_z(newPos.z);
                transformPtr->set_yaw(monster.GetYaw());
            
                auto* monsterMovementPtr = noti.mutable_monster_movement();
            }
        
            SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
            // Broadcast
            room.BroadcastReplication(sendBuffer);
        
            result.sent = true;
            result.handled |= ReplicationDirty::Transform;
        }
    }
    
    return result;
}
