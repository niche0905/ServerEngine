#include "pch.h"
#include "ServiceBase.h"
#include "Network/SocketUtils.h"

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
	if (stopping_.exchange(true))
		return;

	std::vector<std::shared_ptr<SessionBase>> sessions;
	{
		std::lock_guard<std::mutex> lock(sessionMutex_);
		sessions.assign(sessions_.begin(), sessions_.end());
	}

	for (const auto& session : sessions) {
		if (session)
			session->Disconnect(L"Service stopping");
	}
}

void ServiceBase::WaitForSessionDrain()
{
	std::unique_lock<std::mutex> lock(sessionMutex_);
	sessionDrainCv_.wait(lock, [this]() { return sessions_.empty(); });
}

std::shared_ptr<SessionBase> ServiceBase::CreateSession()
{
	if (stopping_.load())
		return nullptr;

	std::shared_ptr<SessionBase> session = sessionFactory_();

	session->SetService(std::static_pointer_cast<ServiceBase>(shared_from_this()));
	if (ShouldRegisterSessionOnCreate()) {
		bool registerSuccess = RegisterSession(session);
		if (not registerSuccess) {
			return nullptr;
		}
	}

	return session;
}

bool ServiceBase::AcceptSession(std::shared_ptr<SessionBase> session, SOCKET socket, const NetAddr& addr)
{
	if (session == nullptr || socket == INVALID_SOCKET)
		return false;

	SocketUtils::Close(session->socket_);
	session->socket_ = socket;
	session->SetNetAddr(addr);

	if (PrepareSessionForConnectedIo(session) == false) {
		SocketUtils::Close(session->socket_);
		return false;
	}

	session->ProcessConnect();
	return true;
}

bool ServiceBase::PrepareSessionForConnectedIo(std::shared_ptr<SessionBase> session)
{
	if (session == nullptr)
		return false;

	return session->PrepareForConnectedIo();
}

bool ServiceBase::AddSession(std::shared_ptr<SessionBase> session)
{
	// TODO: lock free로 효율적인 구현이 가능하다면 변경하도록
	std::lock_guard<std::mutex> lock(sessionMutex_);

	if (stopping_.load())
		return false;

	const bool inserted = sessions_.insert(session).second;
	if (inserted) {
		session->activeRegistered_.store(true);
		++sessionCount_;
	}

	return inserted;
}

void ServiceBase::RemoveSession(std::shared_ptr<SessionBase> session)
{
	{
		std::lock_guard<std::mutex> lock(sessionMutex_);

		assert(sessions_.erase(session) != 0);
		--sessionCount_;
	}

	sessionDrainCv_.notify_all();
}
