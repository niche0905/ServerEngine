#pragma once

class Actor;

struct AttackContext
{
    Actor* instigator = nullptr;   // 공격을 가한 Actor (공격자)
};

enum class AttackType
{
    None,
    Melee,
    Hitscan,
    Projectile,
 };

struct AttackRequest
{
    AttackType type = AttackType::None;
    Actor* instigator = nullptr;   // 공격을 가한 Actor (공격자)
   
    SE::Math::Vector3 origin{};
    SE::Math::Vector3 direction{};
   
    float range = 0.0f;
    float damage = 0.0f;
};
