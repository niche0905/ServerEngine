#pragma once
#include <behaviortree_cpp/action_node.h>

class MonsterPawn;

class MinionReturnToSpawnNode : public BT::StatefulActionNode
{
public:
    MinionReturnToSpawnNode(const std::string& name, const BT::NodeConfiguration& config)
        : StatefulActionNode(name, config) {}

    static BT::PortsList providedPorts();

    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override;

private:
    bool TryMoveToSpawn();

private:
    MonsterPawn* selfNpc_ = nullptr;
    bool hasPath_ = false;
};
