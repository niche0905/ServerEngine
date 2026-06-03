#pragma once
#include <behaviortree_cpp/action_node.h>
#include "Content/Gameplay/Combat/CombatTypes.h"

class MonsterPawn;
class Pawn;

class MinionMeleeAttackNode : public BT::StatefulActionNode
{
public:
    MinionMeleeAttackNode(const std::string& name, const BT::NodeConfiguration& config)
        : StatefulActionNode(name, config) {}

    static BT::PortsList providedPorts();

    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override;

private:
    MonsterPawn* selfNpc_ = nullptr;
    Pawn* targetPawn_ = nullptr;
    float elapsedTime_ = 0.0f;
    bool hitChecked_ = false;
    CombatEventType attackType_ = CombatEventType::None;
};
