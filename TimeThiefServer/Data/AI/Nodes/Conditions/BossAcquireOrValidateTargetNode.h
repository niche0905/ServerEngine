#pragma once
#include <behaviortree_cpp/condition_node.h>

class BossAcquireOrValidateTargetNode : public BT::ConditionNode
{
public:
    BossAcquireOrValidateTargetNode(const std::string& name, const BT::NodeConfiguration& config)
        : ConditionNode(name, config) {}

    static BT::PortsList providedPorts();

    BT::NodeStatus tick() override;
};
