#include "pch.h"
#include "RioEvent.h"

/*------------
   RioEvent
------------*/

RioEvent::RioEvent(IoEventType type)
	: type_(type)
{
	// Nothing but it's safe
	//rioBuffer_ = { RIO_INVALID_BUFFERID, 0, 0 }; or rioBuffer_{ RIO_INVALID_BUFFERID, 0, 0 } in header or ctor initializer
	//context_ = nullptr;
}
