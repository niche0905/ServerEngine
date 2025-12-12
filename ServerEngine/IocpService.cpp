#include "pch.h"
#include "IocpService.h"

/*----------------
   IocpService
----------------*/

IocpService::IocpService(ServiceType type, NetAddr address, SessionFactory factory, int32 maxSessionCount)
	: ServiceBase(type, address, factory, maxSessionCount)
	, iocpCore_(std::make_shared<IocpCore>())
{

}

IocpService::~IocpService()
{
}

bool IocpService::Start()
{
	if (not CanStart()) {
		return false;
	}

	return true;
}

bool IocpService::CanStart() const
{
	if (not ServiceBase::CanStart()) {
		return false;
	}

	if (nullptr == iocpCore_) {
		return false;
	}

	return true;
}

void IocpService::StopService()
{
	ServiceBase::StopService();
}

bool IocpService::RegisterSession(std::shared_ptr<SessionBase> session)
{
	bool succ = iocpCore_->AttachIoObject(session);	// IOCP¿¡ session µî·Ï
	if (not succ) {
		return false;
	}
	
	AddSession(session);
	return true;
}

bool IocpService::RegisterIoObject(std::shared_ptr<IoObject> ioObject)
{
	return iocpCore_->AttachIoObject(ioObject);
}
