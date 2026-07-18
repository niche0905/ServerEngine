#include "pch.h"
#include "BossAcquireOrValidateTargetNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;

namespace
{
    constexpr float AcquireRange = 2500.0f;

    constexpr float LoseRange = 3500.0f;
    constexpr float LoseRangeSq = LoseRange * LoseRange;

    void ResetCombatMode(BT::TreeNode& node)
    {
        node.setOutput<CombatEventType>(BB::CombatMode, CombatEventType::None);
    }

    void ClearTargetState(BT::TreeNode& node, MonsterPawn* selfNpc)
    {
        ResetCombatMode(node);
        node.setOutput<ObjectId>(BB::TargetId, ObjectId{});
        node.setOutput<Pawn*>(BB::TargetPawn, nullptr);

        if (selfNpc) {
            selfNpc->ClearTarget();
        }
    }
}

BT::PortsList BossAcquireOrValidateTargetNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::BidirectionalPort<ObjectId>(BB::TargetId),
        BT::BidirectionalPort<Pawn*>(BB::TargetPawn),
        BT::BidirectionalPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus BossAcquireOrValidateTargetNode::tick()
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

    const bool keepDeadTargetDuringAttack =
        mode == CombatEventType::BossGroundSlam ||
        mode == CombatEventType::BossBurstCharge ||
        mode == CombatEventType::BossBurstChargeStart ||
        mode == CombatEventType::BossBurstExplode;

    Pawn* targetPawn = nullptr;
    if (getInput<Pawn*>(BB::TargetPawn, targetPawn) && targetPawn != nullptr) {
        if (targetPawn->IsDead()) {
            if (keepDeadTargetDuringAttack) {
                return BT::NodeStatus::SUCCESS;
            }

            ClearTargetState(*this, selfNpc);
            return BT::NodeStatus::FAILURE;
        }

        SE::Math::Vector3 diff = targetPawn->GetPosition() - selfNpc->GetPosition();
        diff.z = 0.0f;

        if (diff.LengthSq() <= LoseRangeSq || keepDeadTargetDuringAttack) {
            setOutput<ObjectId>(BB::TargetId, targetPawn->GetId());
            selfNpc->SetTarget(targetPawn);
            return BT::NodeStatus::SUCCESS;
        }

        ClearTargetState(*this, selfNpc);
        return BT::NodeStatus::FAILURE;
    }

    ObjectId targetId{};
    if (getInput<ObjectId>(BB::TargetId, targetId) && targetId != ObjectId{}) {
        Pawn* foundTarget = room->GetObjectManager().FindAs<Pawn>(targetId);
        if (foundTarget != nullptr && !foundTarget->IsDead()) {
            SE::Math::Vector3 diff = foundTarget->GetPosition() - selfNpc->GetPosition();
            diff.z = 0.0f;

            if (diff.LengthSq() <= LoseRangeSq) {
                setOutput<Pawn*>(BB::TargetPawn, foundTarget);
                selfNpc->SetTarget(foundTarget);
                return BT::NodeStatus::SUCCESS;
            }
        }

        ClearTargetState(*this, selfNpc);
    }

    Pawn* bestTarget = selfNpc->SelectTarget(AcquireRange);

    if (bestTarget == nullptr) {
        ResetCombatMode(*this);
        return BT::NodeStatus::FAILURE;
    }

    setOutput<ObjectId>(BB::TargetId, bestTarget->GetId());
    setOutput<Pawn*>(BB::TargetPawn, bestTarget);
    selfNpc->SetTarget(bestTarget);
    return BT::NodeStatus::SUCCESS;
}
