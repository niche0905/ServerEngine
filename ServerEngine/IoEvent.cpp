#include "pch.h"
#include "IoEvent.h"

/*-----------
   IoEvent
-----------*/

IoEvent::IoEvent(IoEventType type)
	: eventType_(type)
	, owner_(nullptr)
{
	Init();
}

void IoEvent::Init()
{
	ZeroMemory(this, sizeof(OVERLAPPED));
}
