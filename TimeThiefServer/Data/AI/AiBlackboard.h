#pragma once
#include <behaviortree_cpp/blackboard.h>

class MonsterPawn;

namespace AiBlackboardKey
{
    inline constexpr const char* SelfNpc                = "self_npc";
    inline constexpr const char* TargetPawn             = "target_pawn";
    inline constexpr const char* TargetId               = "target_id";
    inline constexpr const char* SpawnPos               = "spawn_pos";
    inline constexpr const char* MoveTarget             = "move_target";
}

namespace AiBlackboard
{
    inline MonsterPawn* GetSelfNpc(const BT::Blackboard::Ptr& blackboard)
    {
        MonsterPawn* npc = nullptr;
        if (!blackboard->get(AiBlackboardKey::SelfNpc, npc))
            return nullptr;
        
        return npc;
    }
}

// namespace BB = AiBlackboardKey;      // cpp에서 alias 해서 사용하도록
// namespace BBH = AiBlackboard;
