#include "pch.h"
#include "WorldItemActor.h"

/*-------------------
   WorldItemActor
-------------------*/

void WorldItemActor::InitDrop(const ItemStack& stack, const Vector3& pos, const Vector3& initVel)
{
    stack_ = stack;
    SetPosition(pos);
    velocity_ = initVel;
}

void WorldItemActor::OnSpawn()
{
    Actor::OnSpawn();
}

void WorldItemActor::Tick(float dt)
{
    // Tickable 아님
}
