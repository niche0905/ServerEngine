#include "pch.h"
#include "BaseComponent.h"
#include "Content/Object/BaseObject.h"
#include "Service/Room/Room.h"

/*-----------------
   BaseComponent
-----------------*/

ObjectId BaseComponent::GetOwnerId() const
{
   return owner_ ? owner_->GetId() : ObjectId{};
}

std::shared_ptr<Room> BaseComponent::GetRoom() const
{
   return owner_ ? owner_->GetRoom() : nullptr;
}

ObjectManager* BaseComponent::GetObjectManager() const
{
   auto room = GetRoom();
   return room ? &room->GetObjectManager() : nullptr;
}
