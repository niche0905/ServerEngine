#include "pch.h"
#include "CatDecideMeleeAttackTypeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"

namespace BB = AiBlackboardKey;


BT::PortsList CatDecideMeleeAttackTypeNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::BidirectionalPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus CatDecideMeleeAttackTypeNode::tick()
{
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) || selfNpc == nullptr) {
        return BT::NodeStatus::FAILURE;
    }

    if (selfNpc->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    CombatEventType mode = CombatEventType::None;

    if (getInput<CombatEventType>(BB::CombatMode, mode))
    {
        switch (mode)
        {
        case CombatEventType::CatClaw:
        case CombatEventType::CatBite:
            return BT::NodeStatus::SUCCESS;

        case CombatEventType::CatMelee:
        case CombatEventType::None:
            break;

        default:
            return BT::NodeStatus::FAILURE;
        }
    }

    if (AiBlackboard::RandomChance(0.6f)) {
        setOutput<CombatEventType>(BB::CombatMode, CombatEventType::CatClaw);
    }
    else {
        setOutput<CombatEventType>(BB::CombatMode, CombatEventType::CatBite);
    }

    return BT::NodeStatus::SUCCESS;
}
