#include "pch.h"
#include "PlayerReplicator.h"
#include "Content/Object/Actor/PlayerPawn.h"
#include "Service/Room/Room.h"


ReplicateResult PlayerReplicator::FlushPeriodic(BaseObject* obj, ReplicationDirty flags, const RepFrame& frame, uint64 nowMs,
                                     Room& room) const
{
    auto* player = dynamic_cast<PlayerPawn*>(obj);
    if (!player)
        return ReplicateResult{};
    
    return FlushPlayerPeriodic(*player, flags, frame, nowMs, room);
}

ReplicateResult PlayerReplicator::FlushPlayerPeriodic(PlayerPawn& player, ReplicationDirty flags, const RepFrame& frame,
    uint64 nowMs, Room& room) const
{
    ReplicateResult result;
    
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
            playerMovementPtr->set_aim_yaw((player.GetAimYaw()));
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
        
        result.sent = true;
        result.handled |= ReplicationDirty::Transform;
    }
    if (HasDirty(flags, ReplicationDirty::Health)) {
        // 체력의 delta는 event로 처리하고 여기선 snapshot 느낌의 패킷을 보내도록
        // Health의 최종값 (서버 권위)를 보장하기 위한 방법
        
        se::game::N_HealthSnapshot noti;
        {
            noti.set_current_health(player.GetHealth().GetHp());
            noti.set_max_health(player.GetHealth().GetMaxHp());
        }
        
        SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
        room.SendReplication(player.GetOwnerPlayerId(), sendBuffer);
        
        result.sent = true;
        result.handled |= ReplicationDirty::Health;
    }
    if (HasDirty(flags, ReplicationDirty::Resource)) {
        // 체력과 마찬가지로 서버 권위 구조에서 최종값을 보장하기 위한 패킷
        
        se::game::N_TimePointSnapshot noti;
        {
            noti.set_time_points(player.GetWallet().GetBalance(CurrencyType::TimePoint));
        }
        
        SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
        room.SendReplication(player.GetOwnerPlayerId(), sendBuffer);
        
        result.sent = true;
        result.handled |= ReplicationDirty::Resource;
    }
    if (HasDirty(flags, ReplicationDirty::Inventory)) {
        // Inventory의 경우는 아이템의 추가/제거가 있을 때마다 이벤트로 처리하는 구조로 하는 게 좋을 것 같음
        // Inventory의 snapshot을 보내야 할 필요가 있다면 이걸 활용해서 보내도록 (Respawn 같은 경우)
        
        se::game::N_ItemSnapshot noti;
        {
            auto& inventory = player.GetInventory();
            const auto& items = inventory.GetSlots();
            
            for (const auto& item : items) {
                if (item.IsValid()) {
                    auto* itemInfoPtr = noti.add_items();
                    itemInfoPtr->set_item_id(item.id);
                    itemInfoPtr->set_amount(item.count);
                }
            }
        }
        
        SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
        room.SendReplication(player.GetOwnerPlayerId(), sendBuffer);
        
        result.sent = true;
        result.handled |= ReplicationDirty::Inventory;
    }
    if (HasDirty(flags, ReplicationDirty::Stat)) {
        
        se::game::N_SpeedChanged noti;
        {
            noti.set_new_speed(static_cast<float>(player.GetSpeed()));
        }
        
        SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
        room.SendReplication(player.GetOwnerPlayerId(), sendBuffer);

        result.sent = true;
        result.handled |= ReplicationDirty::Stat;
    }
    if (HasDirty(flags, ReplicationDirty::SkillState)) {
        // 스킬 해금 상태가 변경될 때마다 이벤트로 처리하는 구조로 하는 게 좋을 것 같음
        // Respawn에서도 스킬 해금 상태가 초기화되는 경우가 있을 수 있으니 이걸 활용해서 스킬 해금 상태 패킷 보내도록
        
        se::game::N_SkillUnlockSnapshot noti;
        {
            auto& skillComp = player.GetSkill();
            const auto& unlockedSkills = skillComp.GetUnlockSkills();
            
            for (const auto& skillId : unlockedSkills) {
                noti.add_unlocked_skill_ids(skillId);
            }
            
            const auto& equippedSkills = skillComp.GetEquippedSkills();
            for (int32 slotIndex = 0; slotIndex < MaxActiveSkills; ++slotIndex) {
                SkillId skillId = equippedSkills[slotIndex];
                if (skillId == 0) {
                    continue;
                }
                
                auto* slotPtr = noti.add_equipped_skill_slots();
                slotPtr->set_slot_index(static_cast<uint32>(slotIndex));
                slotPtr->set_skill_id(skillId);
            }
        }
        
        SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
        room.SendReplication(player.GetOwnerPlayerId(), sendBuffer);
        
        result.sent = true;
        result.handled |= ReplicationDirty::SkillState;
    }
    if (HasDirty(flags, ReplicationDirty::WeaponStat)) {
        // 무기 강화 상태가 변경될 때마다 이벤트로 처리하는 구조로 하는 게 좋을 것 같음
        // Respawn에서도 무기 강화 상태가 초기화되는 경우가 있을 수 있으니 이걸 활용해서 무기 강화 상태 패킷 보내도록
        
        se::game::N_WeaponStatSnapshot noti;
        {
            auto* combatComp = player.GetPlayerCombat();
            const auto& weaponStats = combatComp->GetCombatState();
            
            for (const auto& weaponSlotState : weaponStats.slots) {
                auto* weaponSnapshot = noti.add_stats();
                weaponSnapshot->set_weapon_id(weaponSlotState.runtime.weaponId);
                auto* weaponStat = weaponSnapshot->mutable_stat();
                weaponStat->set_mag_capacity(weaponSlotState.stat.common.magCapacity);
                weaponStat->set_fire_interval(weaponSlotState.stat.common.fireIntervalSec);
                weaponStat->set_reload_time(weaponSlotState.stat.common.reloadTimeSec);
                
                if (weaponSlotState.stat.common.category == WeaponCategory::Shotgun) {
                    const ShotgunStat* shotgun = std::get_if<ShotgunStat>(&weaponSlotState.stat.extra);
                    if (shotgun) {
                        weaponStat->set_pellet_count(shotgun->pelletCount);
                        weaponStat->set_cone_angle(shotgun->coneAngleDegrees);
                    }
                }
                else if (weaponSlotState.stat.common.category == WeaponCategory::Launcher) {
                    const LauncherStat* launcher = std::get_if<LauncherStat>(&weaponSlotState.stat.extra);
                    if (launcher) {
                        weaponStat->set_projectile_speed(launcher->projectileSpeed);
                        weaponStat->set_explosion_radius(launcher->explosionRadius);
                    }
                }
            }
        }
        
        SendBufferRef sendBuffer = ServerPacketHandler::MakeSendBuffer(noti);
        room.SendReplication(player.GetOwnerPlayerId(), sendBuffer);
        
        result.sent = true;
        result.handled |= ReplicationDirty::WeaponStat;
    }
    
    return result;
}
