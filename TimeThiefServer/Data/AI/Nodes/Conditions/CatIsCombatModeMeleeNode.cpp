#include "pch.h"
#include "CatIsCombatModeMeleeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;


BT::PortsList CatIsCombatModeMeleeNode::providedPorts()
{
    return {
        BT::InputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatIsCombatModeMeleeNode::tick()
{
    CombatEventType combatMode = CombatEventType::None;

    if (!getInput<CombatEventType>(BB::CombatMode, combatMode)) {
        return BT::NodeStatus::FAILURE;
    }

    switch (combatMode)
    {
    case CombatEventType::CatMelee:
    case CombatEventType::CatClaw:
    case CombatEventType::CatBite:
        return BT::NodeStatus::SUCCESS;

    default:
        return BT::NodeStatus::FAILURE;
    }
}
