#include "pch.h"
#include "RioCore.h"

/*------------
   RioCore
------------*/

bool RioCore::Initialize()
{
	return false;
}

void RioCore::Terminate()
{
}

bool RioCore::Dispatch(DWORD timeoutMs)
{
	return false;
}

bool RioCore::AttachIoObject(std::shared_ptr<IoObject> ioObject)
{
	return false;
}

void RioCore::DetachIoObject(std::shared_ptr<IoObject> ioObject)
{
}
