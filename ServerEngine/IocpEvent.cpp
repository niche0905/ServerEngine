#include "pch.h"
#include "IocpEvent.h"

/*-------------
   IocpEvent
-------------*/

IocpEvent::IocpEvent(IoEventType type)
	: type_(type)
{
	std::memset(static_cast<OVERLAPPED*>(this), 0, sizeof(OVERLAPPED));
}
