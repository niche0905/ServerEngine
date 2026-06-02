#pragma once
#include <behaviortree_cpp/action_node.h>

class Pawn;
class MonsterPawn;

class CatMoveTargetNode : public BT::StatefulActionNode
{
public:
    CatMoveTargetNode(const std::string& name, const BT::NodeConfiguration& config)
        : StatefulActionNode(name, config) {}
    
    static BT::PortsList providedPorts();
    
    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override;
    
private:
    bool TryMoveToGoal(const SE::Math::Vector3& selfPos, const SE::Math::Vector3& goal);
    
private:
    MonsterPawn* selfNpc_ = nullptr;
    Pawn* targetPawn_ = nullptr;

    SE::Math::Vector3 moveGoal_{};
    SE::Math::Vector3 lastTargetPos_{};

    float elapsedRepath_ = 0.0f;
    int32 orbitSide_ = 1;
    
};
