#pragma once
#include "Network/Session/IOCP/IocpSession.h"

/*-----------------
   PlayerSession
-----------------*/
//
// PlayerSession은 게임 플레이어와의 네트워크 세션을 관리합니다.
//

// TODO: Temp로 IocpSession 상속, 추후 필요에 따라 기능 추가 예정
class PlayerSession : public IocpSession
{
public:
   PlayerSession();
   virtual ~PlayerSession();
   
private:
   // TODO: 플레이어 세션 관련 멤버 변수 추가 예정
    
};
