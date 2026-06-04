#include "pch.h"
#include "BossChargeBurstAttackNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Physics/Collider/SphereCollider.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;

namespace
{
    constexpr float AttackAnimDuration = 2.60f;
    constexpr float ExplosionTiming = 1.70f;
    constexpr int32 BurstDamage = 55;
    constexpr float BurstRadius = 750.0f;

    void ResetCombatReservation(BT::TreeNode& node)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
    }
}

BT::PortsList BossChargeBurstAttackNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::OutputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus BossChargeBurstAttackNode::onStart()
{
    elapsedTime_ = 0.0f;
    exploded_ = false;
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

    room->NotifyCombatEvent(selfNpc_->GetId(), CombatEventType::BossBurstChargeStart);
    return BT::NodeStatus::RUNNING;
}

BT::NodeStatus BossChargeBurstAttackNode::onRunning()
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

    if (!exploded_ && !targetPawn_->IsDead()) {
        SE::Math::Vector3 lookDir = targetPawn_->GetPosition() - selfNpc_->GetPosition();
        lookDir.z = 0.0f;
        selfNpc_->LookAtDirection(lookDir);
    }

    elapsedTime_ += room->GetDelta();

    if (!exploded_ && elapsedTime_ >= ExplosionTiming) {
        exploded_ = true;

        room->NotifyCombatEvent(selfNpc_->GetId(), CombatEventType::BossBurstExplode);

        MeleeAttackDesc desc;
        desc.attackerId = selfNpc_->GetId();
        desc.attackType = CombatEventType::BossBurstExplode;
        desc.damage = BurstDamage;
        SE::Physics::SphereCollider collider(selfNpc_->GetPosition() + SE::Math::Vector3{0.0f, 0.0f, 90.0f}, BurstRadius);
        desc.collider = &collider;

        room->HandleMonsterMelee(desc);
    }

    if (elapsedTime_ >= AttackAnimDuration) {
        ResetCombatReservation(*this);
        return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::RUNNING;
}

void BossChargeBurstAttackNode::onHalted()
{
    elapsedTime_ = 0.0f;
    exploded_ = false;
    selfNpc_ = nullptr;
    targetPawn_ = nullptr;
}
