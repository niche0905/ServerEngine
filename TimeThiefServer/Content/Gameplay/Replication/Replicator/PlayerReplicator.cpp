#include "pch.h"
#include "PlayerReplicator.h"
#include "Content/Object/Actor/PlayerPawn.h"


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
    // TODO: PlayerPawn의 Replicate 관련 Dirty flag를 보고 패킷 생성하고 전송하기
    
    if (HasDirty(flags, ReplicationDirty::Transform)) {
        // Move Packet에서 SetPosition, SetYaw 등을 하였을 때 Transform이 Dirty가 되도록 하였기에
        // 이번 Tick에서 바로 동기화
    }
    if (HasDirty(flags, ReplicationDirty::Health)) {
        // 체력의 delta는 event로 처리하고 여기선 snapshot 느낌의 패킷을 보내도록
        // Health의 최종값 (서버 권위)를 보장하기 위한 방법
    }
    if (HasDirty(flags, ReplicationDirty::Resource)) {
        // 체력과 마찬가지로 서버 권위 구조에서 최종값을 보장하기 위한 패킷
    }
    if (HasDirty(flags, ReplicationDirty::Inventory)) {
        // Inventory의 경우는 아이템의 추가/제거가 있을 때마다 이벤트로 처리하는 구조로 하는 게 좋을 것 같음
        // Inventory의 snapshot을 보내야 할 필요가 있다면 이걸 활용해서 보내도록 (Respawn 같은 경우)
    }
    if (HasDirty(flags, ReplicationDirty::SkillState)) {
        // 스킬 해금 상태가 변경될 때마다 이벤트로 처리하는 구조로 하는 게 좋을 것 같음
        // Respawn에서도 스킬 해금 상태가 초기화되는 경우가 있을 수 있으니 이걸 활용해서 스킬 해금 상태 패킷 보내도록
    }
    if (HasDirty(flags, ReplicationDirty::WeaponStat)) {
        // 무기 강화 상태가 변경될 때마다 이벤트로 처리하는 구조로 하는 게 좋을 것 같음
        // Respawn에서도 무기 강화 상태가 초기화되는 경우가 있을 수 있으니 이걸 활용해서 무기 강화 상태 패킷 보내도록
    }
    
    return true;
}
