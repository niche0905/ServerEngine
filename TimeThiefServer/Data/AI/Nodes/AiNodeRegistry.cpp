#include "pch.h"
#include "AiNodeRegistry.h"
#include "Actions/MoveTargetNode.h"
#include "Actions/WaitForTargetNode.h"
#include "Conditions/HaveTargetPlayerNode.h"

namespace AiNodeRegistry
{
    void RegisterAll(BT::BehaviorTreeFactory& factory)
    {
        // factory.registerNodeType<NodeClass>("node_name");
        // ...
        
        // TODO: 위와 같은 방식으로 노드 등록
        
        factory.registerNodeType<HaveTargetPlayerNode>("HaveTargetPlayer");
        factory.registerNodeType<MoveTargetNode>("MoveTarget");
        factory.registerNodeType<WaitForTargetNode>("WaitForTarget");
    }
}
