#include "pch.h"
#include "IocpServerService.h"
#include "Utils/Logger/ConsoleLogger.h"

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

	if (false == listener_->StartListening(shared_from_this(),
		[this](std::shared_ptr<IoObject> ioObject)
			{
				return RegisterIoObject(ioObject);
			})) {
		return false;
	}
	
	consoleLogger->Log(Color::Green, L"[Service] IocpServerService started.\n");

	return true;
}

void IocpServerService::StopService()
{
	if (listener_)
		listener_->CloseListener();
	
	IocpService::StopService();
}
