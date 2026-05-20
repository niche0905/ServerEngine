#pragma once
#include <behaviortree_cpp/action_node.h>

class MonsterPawn;

class CatPatrolAroundSpawnNode : public BT::StatefulActionNode
{
public:
    CatPatrolAroundSpawnNode(const std::string& name, const BT::NodeConfiguration& config)
        : StatefulActionNode(name, config) {}
    
    static BT::PortsList providedPorts();
    
    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override;
    
private:
    MonsterPawn* selfNpc_ = nullptr;

    SE::Math::Vector3 patrolGoal_{};

    float elapsedRepath_ = 0.0f;
    bool hasGoal_ = false;
    
};
