#include "pch.h"
#include "RespawnSystem.h"
#include "Content/Object/ObjectId.h"
#include "Content/Object/Actor/Pawn.h"
#include "Service/Room/Room.h"
#include "Shard/GameShard.h"

/*-----------------
   RespawnSystem
-----------------*/

bool RespawnSystem::Init(Room* ownerRoom)
{
   if (ownerRoom == nullptr)
      return false;
   
   ownerRoom_ = ownerRoom;
   
   return true;
}

void RespawnSystem::RequestRespawn(ObjectId objectId)
{
   if (ownerRoom_ == nullptr)
      return;
   
   Pawn* pawn = ownerRoom_->GetObjectManager().FindAs<Pawn>(objectId);
   if (pawn == nullptr)
      return;   // 오브젝트가 존재하지 않음
   
   if (not pawn->TryReserveRespawn())
      return;  // 리스폰 예약이 불가능한 경우 (예: 재화 부족)
   
   uint64 token = pawn->GetRespawnComponent().GetRespawnToken();
   RoomId roomId = ownerRoom_->GetRoomId();
   GameShard* ownerShard = ownerRoom_->GetOwnerShard();
   if (ownerShard == nullptr)
      return;
   
   Duration respawnDelay = Milliseconds(pawn->GetRespawnComponent().GetDelayMs());
   ownerRoom_->ScheduleAfter(respawnDelay, [ownerShard, roomId, objectId, token]()
   {
      auto room = ownerShard->FindRoom(roomId);
      if (room == nullptr) 
         return;
      
      room->GetRoomGameSystem().GetRespawnSystem().TryExecute(objectId, token);
   });
   
}

void RespawnSystem::TryExecute(ObjectId objectId, uint64 token)
{
   if (ownerRoom_ == nullptr)
      return;
   
   Pawn* pawn = ownerRoom_->GetObjectManager().FindAs<Pawn>(objectId);
   if (pawn == nullptr)
      return;   // 오브젝트가 존재하지 않음
   
   auto& respawnComponent = pawn->GetRespawnComponent();
   if (not respawnComponent.CanExecuteRespawn(token))
      return;
   
   respawnComponent.ExecuteRespawn(ownerRoom_->GetObjectManager(), *pawn);
}
