#pragma once
#include "Content/Enum/InteractTypes.h"

class ObjectManager;

/*-----------------
   IInteractable
-----------------*/
//
// IInteractable는 상호작용 가능한 객체가 구현해야 하는 인터페이스입니다.
// InteractComponent와 같은 컴포넌트를 이용하도록 설계되었습니다.
//

class IInteractable
{
public:
    virtual ~IInteractable() = default;

    // 빠른 거절(거리/락/쿨타임/상태 등)
    virtual InteractResultCode CanInteract(ObjectManager& om, const InteractContext& ctx, const InteractRequest& req) const = 0;

    // 실제 처리(상자 열기, 문 열기, 루팅 시작 등)
    virtual InteractResult Interact(ObjectManager& om, const InteractContext& ctx, const InteractRequest& req) = 0;

    // UI/클라용 힌트
    virtual bool IsInteractableNow() const = 0;
};
