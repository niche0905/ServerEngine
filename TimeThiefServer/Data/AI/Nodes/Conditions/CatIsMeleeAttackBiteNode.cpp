#include "pch.h"
#include "CatIsMeleeAttackBiteNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;


BT::PortsList CatIsMeleeAttackBiteNode::providedPorts()
{
    return {
        BT::InputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatIsMeleeAttackBiteNode::tick()
{
    CombatEventType mode = CombatEventType::None;

    if (!getInput<CombatEventType>(BB::CombatMode, mode)) {
        return BT::NodeStatus::FAILURE;
    }

    return mode == CombatEventType::CatBite
        ? BT::NodeStatus::SUCCESS
        : BT::NodeStatus::FAILURE;
}
