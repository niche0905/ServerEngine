#pragma once
#include "Network/Session/PacketSession.h"
#include "Network/Buffer/SendBuffer.h"
#include "Network/Buffer/RecvBuffer.h"
#include "Network/Buffer/LinearBuffer/LinearBuffer.h"
#include "Network/Buffer/CirculationBuffer/CirculationBuffer.h"

/*---------------
   IocpSession
---------------*/
//
// IocpSession은 IOCP 기반 세션의 기본 클래스입니다
//

class IocpSession : public PacketSession
{
private:
	using ConnectEvent =		IocpConnectEvent;
	using DisconnectEvent =		IocpDisconnectEvent;
	using RecvEvent =			IocpRecvEvent;
	using SendEvent =			IocpSendEvent;

	using RecvBuffer =			LinearBuffer;

public:
	IocpSession();
	virtual ~IocpSession();

// Architecture interface
public:
	virtual bool Connect(SOCKET socket) override;
	virtual void Send(std::shared_ptr<SendBuffer> sendBuffer) override;

// Network interface
public:
	virtual byte* GetRecvBuffer() override;

// Event interface
protected:

	// post is to request
	virtual bool PostConnect() override final;
	virtual bool PostDisconnect() override final;
	virtual void PostRecv() override final;
	virtual void PostSend() override final;

	// process is to complete
	virtual void ProcessConnect() override;
	virtual void ProcessDisconnect() override;
	virtual void ProcessRecv(int32 numOfBytes) override	;
	virtual void ProcessSend(int32 numOfBytes) override;

// on event interface for content override
protected:
	//virtual void OnConnected() override;
	//virtual void OnDisconnected() override;
	//virtual void OnRecvPacket(byte* buffer, int32 len) override;
	//virtual void OnSend(int32 len) override;

private:
	RecvBuffer recvBuffer_;

	std::queue<std::shared_ptr<SendBuffer>> sendQueue_;
	std::atomic<bool> sending_ = false;

private:
	ConnectEvent connectEvent_;
	DisconnectEvent disconnectEvent_;
	RecvEvent recvEvent_;
	SendEvent sendEvent_;

	std::mutex sendMutex_;

};

