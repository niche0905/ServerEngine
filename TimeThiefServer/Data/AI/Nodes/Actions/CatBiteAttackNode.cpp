#include "pch.h"
#include "CatBiteAttackNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Physics/Collider/SphereCollider.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float AttackAnimDuration = 1.20f;
    constexpr float HitTiming = 0.525f;

    constexpr int32 BiteDamage = 40;

    constexpr float BiteAttackRadius = 30.0f;

    void ResetCombatReservation(BT::TreeNode& node)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
    }
}


BT::PortsList CatBiteAttackNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::OutputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatBiteAttackNode::onStart()
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

    room->NotifyCombatEvent(selfNpc_->GetId(), CombatEventType::CatBite);

    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus CatBiteAttackNode::onRunning()
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

        const SE::Math::Vector3 attackPos = selfNpc_->GetPosition();        // TODO: Offset 만큼 이동 시키기

        MeleeAttackDesc desc;
        desc.attackerId = selfNpc_->GetId();
        desc.attackType = CombatEventType::CatBite;
        desc.damage = BiteDamage;
        SE::Physics::SphereCollider collider(attackPos, BiteAttackRadius);
        desc.collider = &collider;

        room->HandleMonsterMelee(desc);
    }

    if (elapsedTime_ >= AttackAnimDuration) {
        ResetCombatReservation(*this);
        return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::RUNNING;
}

void CatBiteAttackNode::onHalted()
{
    elapsedTime_ = 0.0f;
    hitChecked_ = false;

    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
}
