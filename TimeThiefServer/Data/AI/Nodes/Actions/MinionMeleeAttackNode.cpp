#include "pch.h"
#include "MinionMeleeAttackNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Physics/Collider/CapsuleCollider.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float AttackDuration = 0.95f;
    constexpr float HitTiming = 0.35f;
    constexpr int32 AttackDamage = 36;
    constexpr float AttackRadius = 80.0f;

    constexpr SE::Math::Vector3 AttackStartOffset{70.0f, 0.0f, 80.0f};
    constexpr SE::Math::Vector3 AttackEndOffset{175.0f, 0.0f, 80.0f};

    CombatEventType PickAttackType()
    {
        return AiBlackboard::RandomChance(0.5f)
            ? CombatEventType::MinionLeftAttack
            : CombatEventType::MinionRightAttack;
    }

    void ResetCombatMode(BT::TreeNode& node)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
    }
}

BT::PortsList MinionMeleeAttackNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::OutputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus MinionMeleeAttackNode::onStart()
{
    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
    elapsedTime_ = 0.0f;
    hitChecked_ = false;
    attackType_ = CombatEventType::None;

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

    attackType_ = PickAttackType();
    setOutput<CombatEventType>(BB::CombatMode, attackType_);

    selfNpc_->StopMove();
    selfNpc_->LookAtDirection(targetPawn_->GetPosition() - selfNpc_->GetPosition());

    room->NotifyCombatEvent(selfNpc_->GetId(), attackType_);

    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus MinionMeleeAttackNode::onRunning()
{
    if (selfNpc_ == nullptr || targetPawn_ == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc_->GetRoom();
    if (room == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc_->IsDead()) {
        ResetCombatMode(*this);
        return BT::NodeStatus::FAILURE;
    }

    if (!hitChecked_ && !targetPawn_->IsDead())
    {
        SE::Math::Vector3 lookDir = targetPawn_->GetPosition() - selfNpc_->GetPosition();
        lookDir.z = 0.0f;
        selfNpc_->LookAtDirection(lookDir);
    }

    elapsedTime_ += room->GetDelta();

    if (!hitChecked_ && elapsedTime_ >= HitTiming)
    {
        hitChecked_ = true;

        const SE::Math::Vector3 attackStart = selfNpc_->TransformLocalOffsetToWorld(AttackStartOffset);
        const SE::Math::Vector3 attackEnd = selfNpc_->TransformLocalOffsetToWorld(AttackEndOffset);

        MeleeAttackDesc desc;
        desc.attackerId = selfNpc_->GetId();
        desc.attackType = attackType_;
        desc.damage = AttackDamage;

        SE::Physics::CapsuleCollider collider(attackStart, attackEnd, AttackRadius);
        desc.collider = &collider;

        room->HandleMonsterMelee(desc);
    }

    if (elapsedTime_ >= AttackDuration) {
        ResetCombatMode(*this);
        return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::RUNNING;
}

void MinionMeleeAttackNode::onHalted()
{
    elapsedTime_ = 0.0f;
    hitChecked_ = false;
    attackType_ = CombatEventType::None;
    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
}
