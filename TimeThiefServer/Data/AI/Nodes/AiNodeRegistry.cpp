#include "pch.h"
#include "AiNodeRegistry.h"
#include "Actions/CatBiteAttackNode.h"
#include "Actions/CatFrontClawAttackNode.h"
#include "Actions/CatIdleNode.h"
#include "Actions/CatMouthCannonCombatNode.h"
#include "Actions/CatMoveTargetNode.h"
#include "Actions/CatMoveToCannonRangeNode.h"
#include "Actions/CatPatrolAroundSpawnNode.h"
#include "Actions/MinionIdleNode.h"
#include "Actions/MinionMeleeAttackNode.h"
#include "Actions/MinionMoveTargetNode.h"
#include "Actions/MinionReturnToSpawnNode.h"
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
#include "Conditions/MinionAcquireOrValidateTargetNode.h"
#include "Conditions/MinionIsTargetInMeleeRangeNode.h"

namespace AiNodeRegistry
{
    void RegisterAll(BT::BehaviorTreeFactory& factory)
    {
        // factory.registerNodeType<NodeClass>("node_name");
        // ...
        
        // TODO: 위와 같은 방식으로 노드 등록
        
        // Test 노드 등록
        factory.registerNodeType<HaveTargetPlayerNode>("TestHaveTargetPlayer");
        factory.registerNodeType<MoveTargetNode>("TestMoveTarget");
        factory.registerNodeType<WaitForTargetNode>("TestWaitForTarget");
        
        // Cat 노드 등록
        factory.registerNodeType<CatAcquireOrValidateTargetNode>("CatAcquireOrValidateTarget");
        factory.registerNodeType<CatDecideCombatModeNode>("CatDecideCombatMode");
        factory.registerNodeType<CatIsCombatModeCannonNode>("CatIsCombatModeCannon");
        factory.registerNodeType<CatIsCombatModeMeleeNode>("CatIsCombatModeMelee");
        factory.registerNodeType<CatIsTargetInCannonRangeNode>("CatIsTargetInCannonRange");
        factory.registerNodeType<CatIsTargetInMeleeRangeNode>("CatIsTargetInMeleeRange");
        factory.registerNodeType<CatDecideMeleeAttackTypeNode>("CatDecideMeleeAttackType");
        factory.registerNodeType<CatIsMeleeAttackClawNode>("CatIsMeleeAttackClaw");
        factory.registerNodeType<CatIsMeleeAttackBiteNode>("CatIsMeleeAttackBite");
        factory.registerNodeType<CatMouthCannonCombatNode>("CatMouthCannonCombat");
        factory.registerNodeType<CatFrontClawAttackNode>("CatFrontClawAttack");
        factory.registerNodeType<CatBiteAttackNode>("CatBiteAttack");
        factory.registerNodeType<CatMoveToCannonRangeNode>("CatMoveToCannonRange");
        factory.registerNodeType<CatMoveTargetNode>("CatMoveTarget");
        factory.registerNodeType<CatPatrolAroundSpawnNode>("CatPatrolAroundSpawn");
        factory.registerNodeType<CatIdleNode>("CatIdle");

        // Minion 노드 등록
        factory.registerNodeType<MinionAcquireOrValidateTargetNode>("MinionAcquireOrValidateTarget");
        factory.registerNodeType<MinionIsTargetInMeleeRangeNode>("MinionIsTargetInMeleeRange");
        factory.registerNodeType<MinionIdleNode>("MinionIdle");
        factory.registerNodeType<MinionMeleeAttackNode>("MinionMeleeAttack");
        factory.registerNodeType<MinionMoveTargetNode>("MinionMoveTarget");
        factory.registerNodeType<MinionReturnToSpawnNode>("MinionReturnToSpawn");
        
    }
}
