#pragma once
#include <behaviortree_cpp/condition_node.h>

class CatDecideMeleeAttackTypeNode : public BT::ConditionNode
{
public:
    CatDecideMeleeAttackTypeNode(const std::string& name, const BT::NodeConfiguration& config)
        : ConditionNode(name, config) {}
    
    static BT::PortsList providedPorts();
    
    BT::NodeStatus tick() override;
    
};
