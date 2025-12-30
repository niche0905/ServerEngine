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
   
   void Dispatch(class IIoEvent* ioEvent, int32 numOfBytes) override;
   
   // TEMP: 간단한 문자열 통신 테스트
   int32 GetPacketHeaderSize() const override
   {
      return 1;
   }
   int32 GetPacketSize(byte* packet) const override
   {
      return reinterpret_cast<uint8*>(packet)[0];
   }
   
   bool CanPacketProcess(const byte* buffer, int32 len) override;
   void OnRecvPacket(byte* buffer, int32 len) override;
   
private:
   // TODO: 플레이어 세션 관련 멤버 변수 추가 예정
    
};
