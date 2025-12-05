#pragma once
#include "ServiceBase.h"
#include "IocpCore.h"

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
	IocpService(ServiceType type = ServiceType::IocpService, NetAddr address, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~IocpService();

	// Service Start
	virtual bool Start() override;
	virtual bool CanStart() const override;

	// Session Management
	virtual bool RegisterSession(std::shared_ptr<SessionBase> session) override;
	virtual bool RegisterIoObject(std::shared_ptr<IoObject> ioObject) override;

	// IOCP Work
	virtual void DoWork() = 0;

private:
	std::shared_ptr<IocpCore> iocpCore_;

};

