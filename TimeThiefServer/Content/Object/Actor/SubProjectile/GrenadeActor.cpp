#include "pch.h"
#include "GrenadeActor.h"
#include "Service/Room/Room.h"

/*----------------
   GrenadeActor
----------------*/

void GrenadeActor::Explode(ObjectManager& om)
{
   OnExplode(om);
}

void GrenadeActor::OnExplode(ObjectManager& om)
{
   if (auto room = GetRoom()) {
      room->GetRoomGameSystem().GetCombatSystem().ProjectileExplosion(GetId(), GetPosition(), GetOwner(), GetDamage(), explosionRadius_, true);
   }
   
   om.RequestDestroy(GetId());
}
