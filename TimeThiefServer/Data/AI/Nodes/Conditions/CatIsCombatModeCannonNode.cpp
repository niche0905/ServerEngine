#include "pch.h"
#include "CatIsCombatModeCannonNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;


BT::PortsList CatIsCombatModeCannonNode::providedPorts()
{
    return {
        BT::InputPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatIsCombatModeCannonNode::tick()
{
    CombatEventType mode = CombatEventType::None;

    if (!getInput<CombatEventType>(BB::CombatMode, mode)) {
        return BT::NodeStatus::FAILURE;
    }

    if (mode == CombatEventType::CatRange ||
        mode == CombatEventType::CatCannon)
    {
        return BT::NodeStatus::SUCCESS;
    }

    return BT::NodeStatus::FAILURE;
}
