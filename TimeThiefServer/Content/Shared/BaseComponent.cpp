#include "pch.h"
#include "BaseComponent.h"
#include "Content/Object/BaseObject.h"

/*-----------------
   BaseComponent
-----------------*/

ObjectId BaseComponent::GetOwnerId() const
{
   return owner_ ? owner_->GetId() : ObjectId{};
}
