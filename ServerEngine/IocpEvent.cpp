#include "pch.h"
#include "IocpEvent.h"

/*-------------
   IocpEvent
-------------*/

IocpEvent::IocpEvent(IoEventType type)
	: type_(type)
{
	ResetOverlapped();
}
