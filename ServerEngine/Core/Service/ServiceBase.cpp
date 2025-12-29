#include "pch.h"
#include "ServiceBase.h"

/*----------------
   ServiceBase
----------------*/

ServiceBase::ServiceBase(ServiceType type, NetAddr address, SessionFactory factory, int32 maxSessionCount)
	: type_(type)
	, netAddress_(address)
	, sessionFactory_(factory)
	, maxSessionCount_(maxSessionCount)
	, sessionCount_(0)
{

}

ServiceBase::~ServiceBase()
{

}

bool ServiceBase::CanStart() const
{
	return (nullptr != sessionFactory_);
}

void ServiceBase::StopService()
{
	
}

std::shared_ptr<SessionBase> ServiceBase::CreateSession()
{
	std::shared_ptr<SessionBase> session = sessionFactory_();

	session->SetService(std::static_pointer_cast<ServiceBase>(shared_from_this()));
	bool registerSuccess = RegisterSession(session);
	if (not registerSuccess) {
		return nullptr;
	}

	return session;
}

void ServiceBase::AddSession(std::shared_ptr<SessionBase> session)
{
	// TODO: lock free로 효율적인 구현이 가능하다면 변경하도록
	std::lock_guard<std::mutex> lock(sessionMutex_);

	++sessionCount_;
	sessions_.insert(session);
}

void ServiceBase::RemoveSession(std::shared_ptr<SessionBase> session)
{
	std::lock_guard<std::mutex> lock(sessionMutex_);

	assert(sessions_.erase(session) != 0);
	--sessionCount_;
}
