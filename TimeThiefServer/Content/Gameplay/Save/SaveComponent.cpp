#include "pch.h"
#include "SaveComponent.h"

/*-----------------
   SaveComponent
-----------------*/

void SaveComponent::Init(BaseObject* owner)
{
   SetOwner(owner);
}

bool SaveComponent::CaptureSnapshot()
{
   
   return false;
}

bool SaveComponent::Rollback()
{
   
   return false;
}
