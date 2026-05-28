#include "pch.h"
#include "RioServerService.h"
#include "Utils/Logger/ConsoleLogger.h"

/*--------------------
   RioServerService
--------------------*/

RioServerService::RioServerService(NetAddr address, SessionFactory factory, int32 maxSessionCount)
    : RioService(ServiceType::RioServerService, address, factory, maxSessionCount)
    , acceptIocpCore_(std::make_shared<IocpCore>())
{
}

RioServerService::~RioServerService()
{
}

bool RioServerService::Start()
{
    if (false == RioService::Start()) {
        return false;
    }

    listener_ = std::make_shared<Listener>();
    if (nullptr == listener_) {
        return false;
    }

    if (false == listener_->StartListening(shared_from_this(),
        [this](std::shared_ptr<IoObject> ioObject)
        {
            return RegisterAcceptIoObject(ioObject);
        })) {
        return false;
    }
    
    consoleLogger->Log(Color::Green, L"[Service] RioServerService started.\n");

    return true;
}

void RioServerService::StopService()
{
    if (listener_)
        listener_->CloseListener();

    RioService::StopService();
}

bool RioServerService::Dispatch(uint32 timeoutMs)
{
    if (acceptIocpCore_)
        acceptIocpCore_->Dispatch(0);

    return RioService::Dispatch(timeoutMs);
}

bool RioServerService::RegisterAcceptIoObject(std::shared_ptr<IoObject> ioObject)
{
    if (nullptr == acceptIocpCore_) {
        return false;
    }
    
    return acceptIocpCore_->AttachIoObject(ioObject);
}
