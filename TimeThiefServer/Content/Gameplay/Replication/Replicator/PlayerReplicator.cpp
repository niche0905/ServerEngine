#include "pch.h"
#include "PlayerReplicator.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Service/Room/Room.h"


bool PlayerReplicator::FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs,
                                     Room& room) const
{
    auto* player = dynamic_cast<PlayerPawn*>(obj);
    if (!player)
        return false;
    
    return FlushPlayerPeriodic(*player, flags, frame, nowMs, room);
}

bool PlayerReplicator::FlushPlayerPeriodic(PlayerPawn& player, ReplicationDirty flags, const RepFrame& frame,
    uint64 nowMs, Room& room) const
{
    if (HasDirty(flags, ReplicationDirty::Transform)) {
        // Move Packet에서 SetPosition, SetYaw 등을 하였을 때 Transform이 Dirty가 되도록 하였기에
        // 이번 Tick에서 바로 동기화
        
        se::game::N_Move noti;
        {
            auto* entityIdPtr = noti.mutable_entity_id();
            entityIdPtr->set_value(player.GetId().value);
         
            noti.set_object_type(se::common::OBJ_PLAYER);
         
            auto* transformPtr = noti.mutable_transform();
            auto* positionPtr = transformPtr->mutable_position();
            const auto& newPos = player.GetPosition();
            positionPtr->set_x(newPos.x);
            positionPtr->set_y(newPos.y);
            positionPtr->set_z(newPos.z);
            transformPtr->set_yaw(player.GetYaw());
         
            auto* playerMovementPtr = noti.mutable_player_movement();
            playerMovementPtr->set_pitch(player.GetPitch());
            auto* velocityPtr = playerMovementPtr->mutable_velocity();
            const auto& newVelocity = player.GetVelocity();
            velocityPtr->set_x(newVelocity.x);
            velocityPtr->set_y(newVelocity.y);
            playerMovementPtr->set_movement_mode(player.GetMovementMode());
        }
        
        SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
        // 본인 제외 Broadcast
        room.BroadcastReplication(sendBuffer, player.GetOwnerPlayerId());
    }
    if (HasDirty(flags, ReplicationDirty::Health)) {
        // 체력의 delta는 event로 처리하고 여기선 snapshot 느낌의 패킷을 보내도록
        // Health의 최종값 (서버 권위)를 보장하기 위한 방법
        
        // TODO: HealthSetter 패킷 구성하고 호출하기
    }
    if (HasDirty(flags, ReplicationDirty::Resource)) {
        // 체력과 마찬가지로 서버 권위 구조에서 최종값을 보장하기 위한 패킷
        
        // TODO: TimePointSetter 패킷 구성하고 호출하기
    }
    if (HasDirty(flags, ReplicationDirty::Inventory)) {
        // Inventory의 경우는 아이템의 추가/제거가 있을 때마다 이벤트로 처리하는 구조로 하는 게 좋을 것 같음
        // Inventory의 snapshot을 보내야 할 필요가 있다면 이걸 활용해서 보내도록 (Respawn 같은 경우)
        
        // TODO: PlayerRespawn 다음 InventorySnapshot 패킷 구성하고 호출하기
    }
    if (HasDirty(flags, ReplicationDirty::SkillState)) {
        // 스킬 해금 상태가 변경될 때마다 이벤트로 처리하는 구조로 하는 게 좋을 것 같음
        // Respawn에서도 스킬 해금 상태가 초기화되는 경우가 있을 수 있으니 이걸 활용해서 스킬 해금 상태 패킷 보내도록
        
        // TODO: Skill Unlock 패킷 구성하기 (여기서 호출 하는 거 아님 Event로 처리)
        // TODO: Skill UnlockState 패킷 구성하고 호출하기
    }
    if (HasDirty(flags, ReplicationDirty::WeaponStat)) {
        // 무기 강화 상태가 변경될 때마다 이벤트로 처리하는 구조로 하는 게 좋을 것 같음
        // Respawn에서도 무기 강화 상태가 초기화되는 경우가 있을 수 있으니 이걸 활용해서 무기 강화 상태 패킷 보내도록
        
        // TODO: PlayerRespawn 다음 WeaponState 패킷 구성하고 호출하기
    }
    
    return true;
}
