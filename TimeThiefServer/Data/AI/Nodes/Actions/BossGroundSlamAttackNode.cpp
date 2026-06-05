#include "pch.h"
#include "BossGroundSlamAttackNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Physics/Collider/CapsuleCollider.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;

namespace
{
    constexpr float AttackAnimDuration = 3.00f;
    constexpr float HitTiming = 0.93f;
    constexpr int32 SlamDamage = 45;

    constexpr float SlamRadius = 130.0f;
    constexpr SE::Math::Vector3 AttackStartOffset{180.0f, 0.0f, 80.0f};
    constexpr SE::Math::Vector3 AttackEndOffset{950.0f, 0.0f, 80.0f};

    void ResetCombatReservation(BT::TreeNode& node)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
    }
}

BT::PortsList BossGroundSlamAttackNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::OutputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus BossGroundSlamAttackNode::onStart()
{
    elapsedTime_ = 0.0f;
    hitChecked_ = false;
    selfNpc_ = nullptr;
    targetPawn_ = nullptr;

    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc_) || selfNpc_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (!getInput<Pawn*>(BB::TargetPawn, targetPawn_) || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead() || targetPawn_->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    selfNpc_->StopMove();

    SE::Math::Vector3 dir = targetPawn_->GetPosition() - selfNpc_->GetPosition();
    dir.z = 0.0f;
    selfNpc_->LookAtDirection(dir);

    room->NotifyCombatEvent(selfNpc_->GetId(), CombatEventType::BossGroundSlam);
    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus BossGroundSlamAttackNode::onRunning()
{
    if (selfNpc_ == nullptr || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead()) {
        ResetCombatReservation(*this);
        return BT::NodeStatus::FAILURE;
    }

    if (!hitChecked_ && !targetPawn_->IsDead()) {
        SE::Math::Vector3 lookDir = targetPawn_->GetPosition() - selfNpc_->GetPosition();
        lookDir.z = 0.0f;
        selfNpc_->LookAtDirection(lookDir);
    }

    elapsedTime_ += room->GetDelta();

    if (!hitChecked_ && elapsedTime_ >= HitTiming) {
        hitChecked_ = true;

        const SE::Math::Vector3 attackStart = selfNpc_->TransformLocalOffsetToWorld(AttackStartOffset);
        const SE::Math::Vector3 attackEnd = selfNpc_->TransformLocalOffsetToWorld(AttackEndOffset);

        // Debug Draw
        // Room::DebugDrawOptions drawOptions;
        // drawOptions.colorRgba = 0xFF3030FF;
        // drawOptions.duration = 1.0f;
        // drawOptions.thickness = 2.0f;
        // room->NotifyDebugDrawCapsule(attackStart, attackEnd, SlamRadius, drawOptions);

        MeleeAttackDesc desc;
        desc.attackerId = selfNpc_->GetId();
        desc.attackType = CombatEventType::BossGroundSlam;
        desc.damage = SlamDamage;
        SE::Physics::CapsuleCollider collider(attackStart, attackEnd, SlamRadius);
        desc.collider = &collider;

        room->HandleMonsterMelee(desc);
    }

    if (elapsedTime_ >= AttackAnimDuration) {
        ResetCombatReservation(*this);
        return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::RUNNING;
}

void BossGroundSlamAttackNode::onHalted()
{
    elapsedTime_ = 0.0f;
    hitChecked_ = false;
    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
}
