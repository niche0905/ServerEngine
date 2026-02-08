#pragma once
#include "RespawnComponent.h"

class ObjectManager;
class RespawnComponent;
class IRespawnOwner;
struct RespawnContext;

/*------------------
   RespawnService
------------------*/
//
// RespawnService는 리스폰 처리를 담당하는 서비스 클래스입니다.
//

class RespawnService
{
public:
    // THINK: static 멤버로 할지 인스턴스 멤버로 할지 고민
    //        멤버로 respawn 필요한 객체를 가지게 할 수도 있다
    bool TryExecute(ObjectManager& om, RespawnComponent& comp, IRespawnOwner& owner, const RespawnContext& ctx)
    {
        if (not comp.IsDue(ctx.nowMs)) return false;
        
        return comp.ExecuteRespawn(om, owner, ctx);
    }
    
};
