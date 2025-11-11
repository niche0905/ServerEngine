#pragma once
#include "IoCore.h"

/*-------------
   IocpCore
-------------*/
//
// Windows의 IOCP 방식을 사용하는 Core 입니다
//

class IocpCore : public IoCore
{
public:
	IoBackend Backend() const noexcept override { return IoBackend::IOCP; }

	bool Initialize() override;
	void Terminate() override;

	bool Dispatch(DWORD timeoutMs = INFINITE) override;

	bool AttachIoObject(std::shared_ptr<IoObject> ioObject) override;
	void DetachIoObject(std::shared_ptr<IoObject> ioObject) override;

private:
	HANDLE iocpHandle_{ INVALID_HANDLE_VALUE };

};

