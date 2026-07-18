#include "pch.h"
#include "MinionAcquireOrValidateTargetNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;
namespace
{
    constexpr float BoundaryRadius = 4000.0f;
    constexpr float BoundaryRadiusSq = BoundaryRadius * BoundaryRadius;

    constexpr float AggroRange = 1000.0f;

    constexpr float AttackHoldRange = 240.0f;
    constexpr float AttackHoldRangeSq = AttackHoldRange * AttackHoldRange;

    void ClearTargetState(BT::TreeNode& node, MonsterPawn* selfNpc)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
        node.setOutput<ObjectId>(BB::TargetId, ObjectId{});
        node.setOutput<Pawn*>(BB::TargetPawn, nullptr);

        if (selfNpc) {
            selfNpc->ClearTarget();
        }
    }

    bool IsInsideBoundary(const SE::Math::Vector3& spawnPos, const SE::Math::Vector3& pos)
    {
        SE::Math::Vector3 diff = pos - spawnPos;
        diff.z = 0.0f;
        return diff.LengthSq() <= BoundaryRadiusSq;
    }

    bool CanKeepTarget(MonsterPawn* selfNpc, Pawn* targetPawn, CombatEventType mode)
    {
        if (selfNpc == nullptr || targetPawn == nullptr) {
            return false;
        }

        if (targetPawn->IsDead()) {
            return false;
        }

        const SE::Math::Vector3 spawnPos = selfNpc->GetSavedRespawnPosition();
        const SE::Math::Vector3 targetPos = targetPawn->GetPosition();

        if (IsInsideBoundary(spawnPos, targetPos)) {
            return true;
        }

        if (mode == CombatEventType::MinionLeftAttack ||
            mode == CombatEventType::MinionRightAttack)
        {
            SE::Math::Vector3 attackDiff = targetPos - selfNpc->GetPosition();
            attackDiff.z = 0.0f;
            return attackDiff.LengthSq() <= AttackHoldRangeSq;
        }

        return false;
    }
}

BT::PortsList MinionAcquireOrValidateTargetNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::BidirectionalPort<ObjectId>(BB::TargetId),
        BT::BidirectionalPort<Pawn*>(BB::TargetPawn),
        BT::BidirectionalPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus MinionAcquireOrValidateTargetNode::tick()
{
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) || selfNpc == nullptr) {
        ClearTargetState(*this, nullptr);
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc->IsDead()) {
        ClearTargetState(*this, selfNpc);
        return BT::NodeStatus::FAILURE;
    }

    auto room = selfNpc->GetRoom();
    if (room == nullptr) {
        ClearTargetState(*this, selfNpc);
        return BT::NodeStatus::FAILURE;
    }

    CombatEventType mode = CombatEventType::None;
    getInput<CombatEventType>(BB::CombatMode, mode);

    Pawn* targetPawn = nullptr;
    if (getInput<Pawn*>(BB::TargetPawn, targetPawn) && targetPawn != nullptr)
    {
        if (CanKeepTarget(selfNpc, targetPawn, mode))
        {
            setOutput<ObjectId>(BB::TargetId, targetPawn->GetId());
            selfNpc->SetTarget(targetPawn);
            return BT::NodeStatus::SUCCESS;
        }

        ClearTargetState(*this, selfNpc);
        return BT::NodeStatus::FAILURE;
    }

    ObjectId targetId{};
    if (getInput<ObjectId>(BB::TargetId, targetId) && targetId != ObjectId{})
    {
        Pawn* foundTarget = room->GetObjectManager().FindAs<Pawn>(targetId);
        if (CanKeepTarget(selfNpc, foundTarget, mode))
        {
            setOutput<Pawn*>(BB::TargetPawn, foundTarget);
            selfNpc->SetTarget(foundTarget);
            return BT::NodeStatus::SUCCESS;
        }

        ClearTargetState(*this, selfNpc);
    }

    const SE::Math::Vector3 spawnPos = selfNpc->GetSavedRespawnPosition();
    Pawn* bestTarget = selfNpc->SelectTarget(AggroRange, [&](Pawn* pawn)
    {
        return IsInsideBoundary(spawnPos, pawn->GetPosition());
    });

    if (bestTarget == nullptr) {
        setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
        return BT::NodeStatus::FAILURE;
    }

    setOutput<ObjectId>(BB::TargetId, bestTarget->GetId());
    setOutput<Pawn*>(BB::TargetPawn, bestTarget);
    selfNpc->SetTarget(bestTarget);

    return BT::NodeStatus::SUCCESS;
}
