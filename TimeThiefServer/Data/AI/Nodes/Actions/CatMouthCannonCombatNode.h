#pragma once
#include <behaviortree_cpp/action_node.h>

class Pawn;
class MonsterPawn;

class CatMouthCannonCombatNode : public BT::StatefulActionNode
{
public:
    CatMouthCannonCombatNode(const std::string& name, const BT::NodeConfiguration& config)
        : StatefulActionNode(name, config) {}
    
    static BT::PortsList providedPorts();
    
    BT::NodeStatus onStart() override;
    BT::NodeStatus onRunning() override;
    void onHalted() override;
    
private:
    SE::Math::Vector3 MakeConeErrorTargetPosition(
        const SE::Math::Vector3& origin,
        const SE::Math::Vector3& targetPos,
        float radiusAtTarget
    ) const;
    
private:
    float elapsedTime_ = 0.0f;
    bool fired_ = false;
    bool cancelled_ = false;

    MonsterPawn* selfNpc_ = nullptr;
    Pawn* targetPawn_ = nullptr;
    
};
