#pragma once
#include "IocpEvent.h"

class SendBuffer;

/*-----------------
   IocpRecvEvent
-----------------*/

class IocpRecvEvent : public IocpEvent
{
public:
	IocpRecvEvent()
		: IocpEvent(IoEventType::Recv)
	{

	}

};

/*-----------------
   IocpSendEvent
-----------------*/

class IocpSendEvent : public IocpEvent
{
public:
	IocpSendEvent()
		: IocpEvent(IoEventType::Send)
	{

	}

public:
	std::vector<std::shared_ptr<SendBuffer>> sendBuffers_{};

};