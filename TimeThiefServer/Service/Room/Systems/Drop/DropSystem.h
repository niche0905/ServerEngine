#pragma once
#include "Content/Gameplay/Drop/DropTypes.h"
#include "Content/Object/ObjectId.h"

class Room;

/*---------------
   DropSystem
---------------*/
//
// DropSystem는 게임(Room) 내 드롭 관련 로직을 처리하는 시스템 클래스입니다.
//

class DropSystem
{
public:
   DropSystem() = default;
   
   bool Init(Room* ownerRoom);
   
public:
   auto OnEntityDied(ObjectId entityId) -> void;
   DropSpawnResult DropItems(const DropSpawnContext& ctx);
   
private:
   Room* ownerRoom_ = nullptr;   // non-owning
    
};
