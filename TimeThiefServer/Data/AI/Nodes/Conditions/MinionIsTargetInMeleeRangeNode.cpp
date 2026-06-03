#include "pch.h"
#include "MinionIsTargetInMeleeRangeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"

namespace BB = AiBlackboardKey;

BT::PortsList MinionIsTargetInMeleeRangeNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::InputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus MinionIsTargetInMeleeRangeNode::tick()
{
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) || selfNpc == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    Pawn* targetPawn = nullptr;
    if (!getInput<Pawn*>(BB::TargetPawn, targetPawn) || targetPawn == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    CombatEventType mode = CombatEventType::None;
    if (getInput<CombatEventType>(BB::CombatMode, mode)) {
        if (mode == CombatEventType::MinionLeftAttack ||
            mode == CombatEventType::MinionRightAttack)
        {
            return BT::NodeStatus::SUCCESS;
        }
    }

    if (targetPawn->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    constexpr float MeleeRange = 190.0f;
    constexpr float MeleeRangeSq = MeleeRange * MeleeRange;

    SE::Math::Vector3 diff = targetPawn->GetPosition() - SE::Math::Vector3{0.0f, 0.0f, 90.0f} - selfNpc->GetPosition();
    diff.z = 0.0f;

    return diff.LengthSq() <= MeleeRangeSq
        ? BT::NodeStatus::SUCCESS
        : BT::NodeStatus::FAILURE;
}
