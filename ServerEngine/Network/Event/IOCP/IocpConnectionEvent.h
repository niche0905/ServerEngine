#pragma once
#include "IocpEvent.h"

class SessionBase;

/*--------------------
   IocpConnectEvent
--------------------*/

class IocpConnectEvent : public IocpEvent
{
public:
	IocpConnectEvent()
		: IocpEvent(IoEventType::Connect)
	{

	}

};

/*-----------------------
   IocpDisconnectEvent
-----------------------*/

class IocpDisconnectEvent : public IocpEvent
{
public:
	IocpDisconnectEvent()
		: IocpEvent(IoEventType::Disconnect)
	{

	}

};

/*-------------------
   IocpAcceptEvent
-------------------*/

class IocpAcceptEvent : public IocpEvent
{
public:
	IocpAcceptEvent()
		: IocpEvent(IoEventType::Accept)
	{

	}

public:
	std::shared_ptr<SessionBase> session_{ nullptr };

};
