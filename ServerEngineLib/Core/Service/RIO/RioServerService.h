#pragma once
#include "Core/Service/RIO/RioService.h"
#include "Network/Session/Listener.h"
#include "Core/IoCore/IocpCore/IocpCore.h"

/*--------------------
   RioServerService
--------------------*/
//
// RioServerService는 RIO (Registered I/O) 기술을 활용하여 고성능 네트워크 통신을 제공하는 서버 서비스입니다.
//

class Listener;

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
   
   bool RegisterAcceptIoObject(std::shared_ptr<IoObject> ioObject);
   
private:
   std::shared_ptr<IocpCore> acceptIocpCore_{ nullptr };
   std::shared_ptr<Listener> listener_{ nullptr };
    
};
