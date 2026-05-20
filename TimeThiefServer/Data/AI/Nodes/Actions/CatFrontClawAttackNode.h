#pragma once
#include <behaviortree_cpp/action_node.h>

class Pawn;
class MonsterPawn;

class CatFrontClawAttackNode : public BT::StatefulActionNode
{
public:
    CatFrontClawAttackNode(const std::string& name, const BT::NodeConfiguration& config)
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
    
};
