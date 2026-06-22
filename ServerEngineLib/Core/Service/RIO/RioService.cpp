#include "pch.h"
#include "RioService.h"

/*---------------
   RioService
---------------*/

RioService::RioService(ServiceType type, NetAddr address, SessionFactory factory, int32 maxSessionCount)
    : ServiceBase(type, address, factory, maxSessionCount)
    , rioCore_(std::make_shared<RioCore>())
{
}

RioService::~RioService()
{
}

bool RioService::Start()
{
    if (not CanStart()) {
        return false;
    }

    return true;
}

bool RioService::CanStart() const
{
    if (not ServiceBase::CanStart()) {
        return false;
    }
    
    if (nullptr == rioCore_) {
        return false;
    }
    
    return true;
}

void RioService::StopService()
{
    ServiceBase::StopService();
}

bool RioService::Dispatch(uint32 timeoutMs)
{
    return rioCore_->Dispatch(static_cast<DWORD>(timeoutMs));
}

bool RioService::RegisterSession(std::shared_ptr<SessionBase> session)
{
    if (session == nullptr)
        return false;

    if (!rioCore_->AttachIoObject(session))
        return false;

    return true;
}

bool RioService::RegisterIoObject(std::shared_ptr<IoObject> ioObject)
{
    return false;
}
