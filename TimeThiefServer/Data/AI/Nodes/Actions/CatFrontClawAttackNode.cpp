#include "pch.h"
#include "CatFrontClawAttackNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Physics/Collider/CapsuleCollider.h"
#include "Physics/Collider/SphereCollider.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float AttackAnimDuration = 1.10f;
    constexpr float HitTiming = 0.375f;

    constexpr int32 ClawDamage = 13;
    
    constexpr float ClawAttackRadius = 95.0f;
    
    constexpr SE::Math::Vector3 AttackStartOffset{75.0f, 0.0f, 90.0f};
    constexpr SE::Math::Vector3 AttackEndOffset{195.0f, 0.0f, 90.0f};

    void ResetCombatReservation(BT::TreeNode& node)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
    }
}


BT::PortsList CatFrontClawAttackNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::OutputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatFrontClawAttackNode::onStart()
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

    const SE::Math::Vector3 dir = targetPawn_->GetPosition() - selfNpc_->GetPosition();
    selfNpc_->LookAtDirection(dir);

    room->NotifyCombatEvent(selfNpc_->GetId(), CombatEventType::CatClaw);

    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus CatFrontClawAttackNode::onRunning()
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
    
    if (!hitChecked_) {
        SE::Math::Vector3 lookDir = targetPawn_->GetPosition() - selfNpc_->GetPosition();
        lookDir.z = 0.0f;
        selfNpc_->LookAtDirection(lookDir);
    }

    elapsedTime_ += room->GetDelta();

    if (!hitChecked_ && elapsedTime_ >= HitTiming) {
        hitChecked_ = true;

        const SE::Math::Vector3 attackStart = selfNpc_->TransformLocalOffsetToWorld(AttackStartOffset);
        const SE::Math::Vector3 attackEnd = selfNpc_->TransformLocalOffsetToWorld(AttackEndOffset);

        MeleeAttackDesc desc;
        desc.attackerId = selfNpc_->GetId();
        desc.attackType = CombatEventType::CatClaw;
        desc.damage = ClawDamage;
        SE::Physics::CapsuleCollider collider(attackStart, attackEnd, ClawAttackRadius);
        desc.collider = &collider;
            
        room->HandleMonsterMelee(desc);
    }

    if (elapsedTime_ >= AttackAnimDuration) {
        ResetCombatReservation(*this);
        return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::RUNNING;
}

void CatFrontClawAttackNode::onHalted()
{
    if (!hitChecked_ && selfNpc_ != nullptr) {
        // 근접은 별도 Cancel 이벤트가 없다면 굳이 Notify 안 해도 됨.
        // 필요하면 CombatEventType에 CatClawCancel 같은 걸 추가.
    }

    elapsedTime_ = 0.0f;
    hitChecked_ = false;

    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
}
