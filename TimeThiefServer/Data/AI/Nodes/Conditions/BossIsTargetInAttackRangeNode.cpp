#include "pch.h"
#include "BossIsTargetInAttackRangeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"

namespace BB = AiBlackboardKey;

namespace
{
    constexpr float AttackRange = 1000.0f;
    constexpr float AttackRangeSq = AttackRange * AttackRange;
}

BT::PortsList BossIsTargetInAttackRangeNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::InputPort<Pawn*>(BB::TargetPawn),
        BT::InputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus BossIsTargetInAttackRangeNode::tick()
{
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) || selfNpc == nullptr || selfNpc->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    CombatEventType mode = CombatEventType::None;
    if (getInput<CombatEventType>(BB::CombatMode, mode)) {
        if (mode == CombatEventType::BossGroundSlam ||
            mode == CombatEventType::BossBurstCharge ||
            mode == CombatEventType::BossBurstChargeStart ||
            mode == CombatEventType::BossBurstExplode) {
            return BT::NodeStatus::SUCCESS;
        }
    }

    Pawn* targetPawn = nullptr;
    if (!getInput<Pawn*>(BB::TargetPawn, targetPawn) || targetPawn == nullptr || targetPawn->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    SE::Math::Vector3 diff = targetPawn->GetPosition() - selfNpc->GetPosition();
    diff.z = 0.0f;

    return diff.LengthSq() <= AttackRangeSq
        ? BT::NodeStatus::SUCCESS
        : BT::NodeStatus::FAILURE;
}
