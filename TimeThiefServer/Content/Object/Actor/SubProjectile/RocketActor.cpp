#include "pch.h"
#include "RocketActor.h"

/*---------------
   RocketActor
---------------*/

void RocketActor::OnHit(ObjectManager& om, ObjectId hitObjectId, const SE::Physics::Hit::HitResult& hit)
{
    ProjectileActor::OnHit(om, hitObjectId, hit);
 
    OnExplode(om);
}
