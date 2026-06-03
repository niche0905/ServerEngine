#pragma once
#include <behaviortree_cpp/action_node.h>

class MonsterPawn;
class Pawn;

class MinionMoveTargetNode : public BT::StatefulActionNode
{
public:
    MinionMoveTargetNode(const std::string& name, const BT::NodeConfiguration& config)
        : StatefulActionNode(name, config) {}

    static BT::PortsList providedPorts();

    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override;

private:
    bool TryMoveToGoal(const SE::Math::Vector3& goal);

private:
    MonsterPawn* selfNpc_ = nullptr;
    Pawn* targetPawn_ = nullptr;
    SE::Math::Vector3 lastTargetPos_{};
    float elapsedRepath_ = 0.0f;
};
