#pragma once
#include "Core/Service/RIO/RioService.h"

/*--------------------
   RioServerService
--------------------*/
//
// RioServerService는 RIO (Registered I/O) 기술을 활용하여 고성능 네트워크 통신을 제공하는 서버 서비스입니다.
//

class RioServerService : public RioService
{
public:
   RioServerService() = delete;
   RioServerService(NetAddr address, SessionFactory factory, int32 maxSessionCount = 1);
   virtual ~RioServerService();

   // Service Start
   virtual bool Start() override;
   virtual void StopService() override;
   
   virtual bool Dispatch(uint32 timeoutMs) override;
   
private:
   bool StartListening();
   void AcceptLoop();

private:
   SOCKET listenSocket_{ INVALID_SOCKET };
   std::thread acceptThread_{};
   std::atomic<bool> acceptRunning_{ false };
    
};
