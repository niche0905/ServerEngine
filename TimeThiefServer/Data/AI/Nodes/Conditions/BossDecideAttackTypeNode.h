#pragma once
#include <behaviortree_cpp/condition_node.h>

class BossDecideAttackTypeNode : public BT::ConditionNode
{
public:
    BossDecideAttackTypeNode(const std::string& name, const BT::NodeConfiguration& config)
        : ConditionNode(name, config) {}

    static BT::PortsList providedPorts();

    BT::NodeStatus tick() override;
};
