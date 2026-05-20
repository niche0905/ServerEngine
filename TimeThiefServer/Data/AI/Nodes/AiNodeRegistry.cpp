#include "pch.h"
#include "AiNodeRegistry.h"
#include "Actions/CatBiteAttackNode.h"
#include "Actions/CatFrontClawAttackNode.h"
#include "Actions/CatIdleNode.h"
#include "Actions/CatMouthCannonCombatNode.h"
#include "Actions/CatMoveTargetNode.h"
#include "Actions/CatMoveToCannonRangeNode.h"
#include "Actions/CatPatrolAroundSpawnNode.h"
#include "Actions/MoveTargetNode.h"
#include "Actions/WaitForTargetNode.h"
#include "Conditions/CatAcquireOrValidateTargetNode.h"
#include "Conditions/CatDecideCombatModeNode.h"
#include "Conditions/CatDecideMeleeAttackTypeNode.h"
#include "Conditions/CatIsCombatModeCannonNode.h"
#include "Conditions/CatIsCombatModeMeleeNode.h"
#include "Conditions/CatIsMeleeAttackBiteNode.h"
#include "Conditions/CatIsMeleeAttackClawNode.h"
#include "Conditions/CatIsTargetInCannonRangeNode.h"
#include "Conditions/CatIsTargetInMeleeRangeNode.h"
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
        
        // Cat 노드 등록
        factory.registerNodeType<CatAcquireOrValidateTargetNode>("AcquireOrValidateTarget");
        factory.registerNodeType<CatDecideCombatModeNode>("DecideCombatMode");
        factory.registerNodeType<CatIsCombatModeCannonNode>("IsCombatModeCannon");
        factory.registerNodeType<CatIsCombatModeMeleeNode>("IsCombatModeMelee");
        factory.registerNodeType<CatIsTargetInCannonRangeNode>("IsTargetInCannonRange");
        factory.registerNodeType<CatIsTargetInMeleeRangeNode>("IsTargetInMeleeRange");
        factory.registerNodeType<CatDecideMeleeAttackTypeNode>("DecideMeleeAttackType");
        factory.registerNodeType<CatIsMeleeAttackClawNode>("IsMeleeAttackClaw");
        factory.registerNodeType<CatIsMeleeAttackBiteNode>("IsMeleeAttackBite");
        factory.registerNodeType<CatMouthCannonCombatNode>("MouthCannonCombat");
        factory.registerNodeType<CatFrontClawAttackNode>("FrontClawAttack");
        factory.registerNodeType<CatBiteAttackNode>("BiteAttack");
        factory.registerNodeType<CatMoveToCannonRangeNode>("MoveToCannonRange");
        factory.registerNodeType<CatMoveTargetNode>("MoveTarget");
        factory.registerNodeType<CatPatrolAroundSpawnNode>("PatrolAroundSpawn");
        factory.registerNodeType<CatIdleNode>("Idle");
        
    }
}
