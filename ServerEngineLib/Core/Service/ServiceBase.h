#pragma once
#include <condition_variable>
#include "Network/IoBackend.h"
#include "Network/Session/SessionBase.h"

/*----------------
   ServiceBase
----------------*/
//
// ServiceBase는 Service의 기본 인터페이스입니다
// 다양한 종류의 Service가 이 클래스를 상속받아 구현됩니다
//

class SessionBase;
using SessionFactory = std::function<std::shared_ptr<SessionBase>(void)>;

class ServiceBase : public std::enable_shared_from_this<ServiceBase>
{
public:
	ServiceBase() = delete;
	ServiceBase(ServiceType type, NetAddr address, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~ServiceBase();

public:
	// Service Start
	virtual bool Start() = 0;
	virtual bool CanStart() const;

	virtual void StopService();
	void WaitForSessionDrain();
	
	virtual bool Dispatch(uint32 timeoutMs) = 0;

	// Session Factory Setting
	void SetSessionFactory(SessionFactory factory) { sessionFactory_ = factory; }

	// Session Management
	std::shared_ptr<SessionBase> CreateSession();
	bool AcceptSession(std::shared_ptr<SessionBase> session, SOCKET socket, const NetAddr& addr);
	virtual bool PrepareSessionForConnectedIo(std::shared_ptr<SessionBase> session);
	virtual bool RegisterSession(std::shared_ptr<SessionBase> session) = 0;
	virtual bool RegisterIoObject(std::shared_ptr<IoObject> session) = 0;
	bool AddSession(std::shared_ptr<SessionBase> session);
	void RemoveSession(std::shared_ptr<SessionBase> session);
	int32 GetCurrentSessionCount() const { return sessionCount_; }
	int32 GetMaxSessionCount() const { return maxSessionCount_; }

// Service Information
public:
	ServiceType GetServiceType() const { return type_; }
	NetAddr GetNetAddress() const { return netAddress_; }

protected:
	virtual bool ShouldRegisterSessionOnCreate() const { return true; }

	ServiceType type_;									// what kind of service
	NetAddr netAddress_;								// network address info

	SessionFactory sessionFactory_;						// session factory function
	int32 maxSessionCount_;								// maximum session count
	std::atomic<int32> sessionCount_;					// current session count
	std::set<std::shared_ptr<SessionBase>> sessions_;	// active sessions
	std::atomic<bool> stopping_{ false };

	std::mutex sessionMutex_;							// mutex for session management
	std::condition_variable sessionDrainCv_;

};

