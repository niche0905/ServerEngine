#include "pch.h"
#include "CatIsMeleeAttackClawNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;


BT::PortsList CatIsMeleeAttackClawNode::providedPorts()
{
    return {
        BT::InputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatIsMeleeAttackClawNode::tick()
{
    CombatEventType mode = CombatEventType::None;

    if (!getInput<CombatEventType>(BB::CombatMode, mode)) {
        return BT::NodeStatus::FAILURE;
    }

    return mode == CombatEventType::CatClaw
        ? BT::NodeStatus::SUCCESS
        : BT::NodeStatus::FAILURE;
}
