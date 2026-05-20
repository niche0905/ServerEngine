#pragma once
#include <random>
#include <behaviortree_cpp/blackboard.h>

class MonsterPawn;

namespace AiBlackboardKey
{
    inline constexpr const char* SelfNpc                = "self_npc";
    inline constexpr const char* TargetPawn             = "target_pawn";
    inline constexpr const char* TargetId               = "target_id";
    inline constexpr const char* CombatMode             = "combat_mode";
    
}

namespace AiBlackboard
{
    inline bool RandomChance(float probability)
    {
        static thread_local std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
        
        return distribution(rng) < probability;
    }
}

// namespace BB = AiBlackboardKey;      // cpp에서 alias 해서 사용하도록
// namespace BBH = AiBlackboard;
