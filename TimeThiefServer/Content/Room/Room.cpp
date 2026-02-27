#include "pch.h"
#include "Room.h"
#include "Content/Object/BaseObject.h"

/*---------
   Room
---------*/

// TEMP: 임시로 전역으로 생성, 추후 Service와 Thread와 맞게 구조 변경 필요
std::shared_ptr<Room> GRoom = std::make_shared<Room>(1);

Room::Room(uint32 roomId)
   : roomId_(roomId)
   , objectManager_(roomId)
{
}

Room::~Room()
{
}

bool Room::EnterRoom(std::shared_ptr<BaseObject> object)
{
   bool success = AddObject(object);
   
   // TODO: Player 먼저
}

bool Room::LeaveRoom(std::shared_ptr<BaseObject> object)
{
   if (object == nullptr) return false;
   
   const ObjectId id = object->GetId();
   bool success = RemoveObject(id);
   
   // TODO: Player 먼저
}

void Room::UpdateTick()
{
}

std::shared_ptr<Room> Room::GetRoomRef()
{
   return static_pointer_cast<Room>(shared_from_this());
}
