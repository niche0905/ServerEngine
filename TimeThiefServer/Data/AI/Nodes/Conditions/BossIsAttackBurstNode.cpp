#include "pch.h"
#include "BossIsAttackBurstNode.h"
#include "Content/Gameplay/Combat/CombatTypes.h"
#include "Data/AI/AiBlackboard.h"

namespace BB = AiBlackboardKey;

BT::PortsList BossIsAttackBurstNode::providedPorts()
{
    return {
        BT::InputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus BossIsAttackBurstNode::tick()
{
    CombatEventType mode = CombatEventType::None;
    if (!getInput<CombatEventType>(BB::CombatMode, mode)) {
        return BT::NodeStatus::FAILURE;
    }

    return mode == CombatEventType::BossBurstCharge
        ? BT::NodeStatus::SUCCESS
        : BT::NodeStatus::FAILURE;
}
