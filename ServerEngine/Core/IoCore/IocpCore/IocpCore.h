#pragma once
#include "IoCore.h"
#include "IoBackend.h"

/*-------------
   IocpCore
-------------*/
//
// Windows의 IOCP 방식을 사용하는 Core 입니다
//

class IocpCore : public IoCore
{
public:
	BackendType Backend() const noexcept override { return BackendType::IOCP; }

	bool Initialize() override;
	void Terminate() override;

	bool Dispatch(DWORD timeoutMs = INFINITE) override;

	bool AttachIoObject(std::shared_ptr<IoObject> ioObject) override;

private:
	HANDLE iocpHandle_{ INVALID_HANDLE_VALUE };

};

