#include "pch.h"
#include "RocketActor.h"

/*---------------
   RocketActor
---------------*/

void RocketActor::OnHit(ObjectManager& om, ObjectId hitObjectId)
{
    ProjectileActor::OnHit(om, hitObjectId);
 
    OnExplode(om);
}
