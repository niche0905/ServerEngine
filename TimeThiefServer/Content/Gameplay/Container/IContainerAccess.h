#pragma once
#include "Content/Object/ObjectId.h"

class ObjectManager;
class PlayerPawn;
struct ItemStack;

struct ContainerOpenResult
{
    bool ok{false};
    int32 errorCode{0};
};

struct ContainerTakeResult
{
    bool ok{false};
    int32 errorCode{0};
    int32 moveCount{0};
};

/*--------------------
   IContainerAccess
--------------------*/
//
// IContinerAccess는 컨테이너 접근을 위한 인터페이스입니다.
// 컨테이너에 아이템을 넣거나 빼는 등의 작업을 수행할 수 있습니다.
// 컨테이너에 대한 접근이 유효한지 확인하는 메서드를 포함합니다.
//

class IContainerAccess
{
public:
    ~IContainerAccess() = default;
    
    virtual ContainerOpenResult TryOpen(ObjectManager& om, PlayerPawn& byPlayer) = 0;
    virtual void Close(ObjectManager& om, PlayerPawn& byPlayer) = 0;
    
    // 특정 슬롯에서 아이템을 꺼내는 시도
    virtual ContainerTakeResult TryTakeFromContainer(
        ObjectManager& om, 
        PlayerPawn& byPlayer, 
        int32 containerSlot, 
        int32 takeCount) = 0;
    
    // 컨테이너의 모든 아이템을 꺼내는 시도
    virtual ContainerTakeResult TryTakeAll(ObjectManager& om, PlayerPawn& byPlayer) = 0;
    
    virtual bool IsEmpty() const = 0;
    
};
