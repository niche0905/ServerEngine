#pragma once
#include "IoObject.h"
#include "NetworkAddress.h"
#include "ServiceBase.h"

/*---------------
   SessionBase
---------------*/
//
// SessionBase는 세션의 기본 클래스입니다
//

class SessionBase : public IoObject
{
	friend class Listener;

	enum
	{
		BUFFER_SIZE = 0x1000,	// 4KB
	};

public:
	SessionBase();
	virtual ~SessionBase();

// Architecture interface
public:
	// TODO: void Send()... 어떻게 분리해야 옳지?
	virtual bool Connect(SOCKET socket) = 0;
	void Disconnect(std::wstring_view cause);

	std::shared_ptr<ServiceBase> GetService() { return service_.lock(); }
	void SetService(std::shared_ptr<ServiceBase> service) { service_ = service; }

	virtual std::shared_ptr<SessionBase> GetSessionRef()
		{ return std::static_pointer_cast<SessionBase>(shared_from_this()); }

// Network interface
public:
	const NetAddr& GetNetAddr() const { return netAddr_; }
	void SetNetAddr(const NetAddr& addr) { netAddr_ = addr; }

	SOCKET GetSocket() { return socket_; }
	bool IsConnected() const { return connected_; }

	virtual BYTE* GetRecvBuffer() = 0;

// Event interface
protected:
	
	// post is to request
	virtual bool PostConnect() = 0;
	virtual bool PostDisconnect() = 0;
	virtual void PostRecv() = 0;
	virtual void PostSend() = 0;

	// process is to complete
	void ProcessConnect();
	void ProcessDisconnect();
	virtual void ProcessRecv(int32 numOfBytes) = 0;
	virtual void ProcessSend(int32 numOfBytes) = 0;


// Error handling
protected:
	void HandleError(std::wstring_view functionName, int32 errorCode);

// on event interface for content override
protected:
	virtual void OnConnected() {};
	virtual void OnDisconnected() {};
	virtual int32 OnRecv(BYTE* buffer, int32 len) { return len; }
	virtual void OnSend(int32 len) {}

protected:
	std::weak_ptr<ServiceBase>	service_{};
	SOCKET						socket_{ INVALID_SOCKET };
	NetAddr						netAddr_{};

	std::atomic<bool>			connected_{ false };

};

