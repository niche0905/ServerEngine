#pragma once
#include <behaviortree_cpp/condition_node.h>

class CatIsMeleeAttackClawNode : public BT::ConditionNode
{
public:
    CatIsMeleeAttackClawNode(const std::string& name, const BT::NodeConfiguration& config)
        : ConditionNode(name, config) {}
    
    static BT::PortsList providedPorts();
    
    BT::NodeStatus tick() override;
    
};
