#pragma once
#include "Core/Service/ServiceBase.h"
#include "Core/IoCore/IocpCore/IocpCore.h"

/*----------------
   IocpService
----------------*/
//
// IocpService는 IOCP 기반의 Service 인터페이스입니다
//

class IocpService : public ServiceBase
{
public:
	IocpService() = delete;
	IocpService(ServiceType type, NetAddr address, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~IocpService();

	// Service Start
	virtual bool Start() override;
	virtual bool CanStart() const override;

	virtual void StopService() override;
	
	virtual bool Dispatch(uint32 timeoutMs) override;

	// Session Management
	virtual bool RegisterSession(std::shared_ptr<SessionBase> session) override;
	virtual bool RegisterIoObject(std::shared_ptr<IoObject> ioObject) override;

protected:
	std::shared_ptr<IocpCore> iocpCore_;

};

