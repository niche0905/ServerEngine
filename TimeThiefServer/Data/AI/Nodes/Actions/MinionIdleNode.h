#pragma once
#include <behaviortree_cpp/action_node.h>

class MonsterPawn;

class MinionIdleNode : public BT::StatefulActionNode
{
public:
    MinionIdleNode(const std::string& name, const BT::NodeConfiguration& config)
        : StatefulActionNode(name, config) {}

    static BT::PortsList providedPorts();

    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override;

private:
    MonsterPawn* selfNpc_ = nullptr;
    float elapsedTime_ = 0.0f;
    float idleDuration_ = 0.0f;
};
