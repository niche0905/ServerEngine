#pragma once

struct ObjectId;
class Room;

/*-----------------
   RespawnSystem
-----------------*/
//
// RespawnSystem는 Pawn이 사망한 후 일정 시간 후에 다시 게임에 참여할 수 있도록 하는 시스템입니다.
// Room에 배치되어 Pawn이 사망하였을 때 RespawnSystem에서 Timer에 접근하여 예약하는 방식입니다.
//

class RespawnSystem
{
public:
   RespawnSystem() = default;
   
   bool Init(Room* ownerRoom);
   bool RequestRespawn(ObjectId objectId);
   void TryExecute(ObjectId objectId, uint64 token);
   
private:
   Room* ownerRoom_ = nullptr;   // non-owning
    
};
