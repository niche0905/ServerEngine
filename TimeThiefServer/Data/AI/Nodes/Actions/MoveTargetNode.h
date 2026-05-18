#pragma once
#include <behaviortree_cpp/action_node.h>

class MoveTargetNode : public BT::SyncActionNode
{
public:
    MoveTargetNode(const std::string& name, const BT::NodeConfiguration& config)
        : SyncActionNode(name, config) {}
    
    static BT::PortsList providedPorts()
    {
        return {};
    }
    
    BT::NodeStatus tick() override;
};
