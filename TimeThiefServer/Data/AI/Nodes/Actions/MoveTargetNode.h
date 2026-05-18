#pragma once
#include <behaviortree_cpp/action_node.h>

class MoveTargetNode : public BT::StatefulActionNode
{
public:
    MoveTargetNode(const std::string& name, const BT::NodeConfiguration& config)
        : StatefulActionNode(name, config) {}
    
    static BT::PortsList providedPorts();
    
    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override;
    
};
