#include "pch.h"
#include "IocpCore.h"

/*-------------
   IocpCore
-------------*/

bool IocpCore::Initialize()
{
	iocpHandle_ = ::CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	return (iocpHandle_ != INVALID_HANDLE_VALUE);
}

void IocpCore::Terminate()
{
	::CloseHandle(iocpHandle_);
}

bool IocpCore::Dispatch(DWORD timeoutMs)
{
	// TODO: Seession과 IoObject가 구현이 되어야 구현 가능

	return false;
}

bool IocpCore::AttachIoObject(std::shared_ptr<IoObject> ioObject)
{
	//return ::CreateIoCompletionPort()
	return false;
}

void IocpCore::DetachIoObject(std::shared_ptr<IoObject> ioObject)
{

}
