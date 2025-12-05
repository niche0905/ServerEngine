#include "pch.h"
#include "IocpServerService.h"

/*---------------------
   IocpServerService
---------------------*/

IocpServerService::IocpServerService(NetAddr address, SessionFactory factory, int32 maxSessionCount)
	: IocpService{ ServiceType::IocpServerService, address, factory, maxSessionCount }
{
}

IocpServerService::~IocpServerService()
{
}

bool IocpServerService::Start()
{
	if (false == IocpService::Start()) {
		return false;
	}

	listener_ = std::make_shared<Listener>();
	if (nullptr == listener_) {
		return false;
	}

	if (false == listener_->StartListening(shared_from_this())) {
		return false;
	}

	return true;
}
