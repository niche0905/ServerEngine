#pragma once
#include "Core/Service/ServiceBase.h"
#include "Core/IoCore/RioCore/RioCore.h"

/*---------------
   RioService
---------------*/
//
// RioService는 RIO 기반의 Service 인터페이스입니다
//

class RioService : public ServiceBase
{
public:
   RioService() = delete;
   RioService(ServiceType type, NetAddr address, SessionFactory factory, int32 maxSessionCount = 1);
   virtual ~RioService();

   virtual bool Start() override;
   virtual bool CanStart() const override;
   
   virtual void StopService() override;

   virtual bool Dispatch(uint32 timeoutMs) override;

	// Session Management
   virtual bool RegisterSession(std::shared_ptr<SessionBase> session) override;
   virtual bool RegisterIoObject(std::shared_ptr<IoObject> ioObject) override;

protected:
   std::shared_ptr<RioCore> rioCore_;
    
};
