#include "pch.h"
#include "BossIsAttackSlamNode.h"
#include "Content/Gameplay/Combat/CombatTypes.h"
#include "Data/AI/AiBlackboard.h"

namespace BB = AiBlackboardKey;

BT::PortsList BossIsAttackSlamNode::providedPorts()
{
    return {
        BT::InputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus BossIsAttackSlamNode::tick()
{
    CombatEventType mode = CombatEventType::None;
    if (!getInput<CombatEventType>(BB::CombatMode, mode)) {
        return BT::NodeStatus::FAILURE;
    }

    return mode == CombatEventType::BossGroundSlam
        ? BT::NodeStatus::SUCCESS
        : BT::NodeStatus::FAILURE;
}
