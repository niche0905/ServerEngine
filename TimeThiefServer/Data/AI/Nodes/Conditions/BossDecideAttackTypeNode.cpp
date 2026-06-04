#include "pch.h"
#include "BossDecideAttackTypeNode.h"
#include "Data/AI/AiBlackboard.h"
#include "Content/Object/Actor/MonsterPawn.h"

namespace BB = AiBlackboardKey;

BT::PortsList BossDecideAttackTypeNode::providedPorts()
{
    return {
        BT::InputPort<MonsterPawn*>(BB::SelfNpc),
        BT::BidirectionalPort<CombatEventType>(BB::CombatMode)
    };
}

BT::NodeStatus BossDecideAttackTypeNode::tick()
{
    MonsterPawn* selfNpc = nullptr;
    if (!getInput<MonsterPawn*>(BB::SelfNpc, selfNpc) || selfNpc == nullptr || selfNpc->IsDead()) {
        return BT::NodeStatus::FAILURE;
    }

    CombatEventType mode = CombatEventType::None;
    if (getInput<CombatEventType>(BB::CombatMode, mode)) {
        switch (mode) {
        case CombatEventType::BossGroundSlam:
        case CombatEventType::BossBurstCharge:
            return BT::NodeStatus::SUCCESS;
        case CombatEventType::None:
            break;
        default:
            return BT::NodeStatus::FAILURE;
        }
    }

    if (AiBlackboard::RandomChance(0.55f)) {
        setOutput<CombatEventType>(BB::CombatMode, CombatEventType::BossGroundSlam);
    }
    else {
        setOutput<CombatEventType>(BB::CombatMode, CombatEventType::BossBurstCharge);
    }

    return BT::NodeStatus::SUCCESS;
}
