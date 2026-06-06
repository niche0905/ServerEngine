#pragma once
#include "TypesDef.h"
#include <unordered_map>

struct SkillDef
{
    SkillId skillId{0};
    uint32 cooldownMs{0};
    uint32 durationMs{0};
    uint32 cooldownGroupId{0};
};

struct SkillTable
{
    std::unordered_map<SkillId, SkillDef> skills;

    const SkillDef* Find(SkillId skillId) const
    {
        auto it = skills.find(skillId);
        return it == skills.end() ? nullptr : &it->second;
    }

    bool Contains(SkillId skillId) const
    {
        return skills.contains(skillId);
    }
};
