#pragma once
#include "Content/Object/ObjectManager.h"

class BaseObject;
class ObjectManager;
class Player;

/*---------
   Room
---------*/
//
// Room는 게임 플레이어들이 함께 상호작용하는 공간을 나타냅니다. 
// 각 Room은 고유한 ID를 가지며, 플레이어와 게임 오브젝트들이 존재할 수 있습니다. 
// Room은 게임 로직과 상태를 관리하며, 플레이어 간의 상호작용을 중개하는 역할을 합니다.
//

class Room : public std::enable_shared_from_this<Room>
{
public:
   explicit Room(uint32 roomId);
   ~Room();
   
public:
   bool EnterRoom(std::shared_ptr<BaseObject> object);
   bool LeaveRoom(std::shared_ptr<BaseObject> object);
   
   // TODO: Player 객체 만들기
   bool HandleEnterPlayer(std::shared_ptr<Player> player);
   bool HandleLeavePlayer(std::shared_ptr<Player> player);
   
public:
   void UpdateTick();
   
   std::shared_ptr<Room> GetRoomRef();
   
private:
   bool AddObject(std::shared_ptr<BaseObject> object);
   bool RemoveObject(ObjectId objectId);
   
private:
   void Broadcast(std::shared_ptr<SendBuffer>, uint32 exceptId = 0);
   
private:
   uint32 roomId_;
   ObjectManager objectManager_;
    
};

extern std::shared_ptr<Room> GRoom;
