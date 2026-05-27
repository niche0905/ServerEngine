#pragma once
#include "Core/IoCore/IoCore.h"
#include "Network/IoBackend.h"

/*------------
   RioCore
------------*/
//
// Windows의 Registered I/O (RIO) 방식을 사용하는 Core 입니다
//

class RioCore : public IoCore
{
public:
	RioCore();
	~RioCore() override;
	
public:
	BackendType Backend() const noexcept override { return BackendType::RIO; }
	
	bool Initialize() override;
	void Terminate() override;
	
	bool Dispatch(DWORD timeoutMs = INFINITE) override;
	
	bool AttachIoObject(std::shared_ptr<IoObject> ioObject) override;

public:
	// CQ 핸들 접근자
	RIO_CQ GetCompletionQueue() const noexcept { return rioCq_; }

private:
	// Completion Queue 핸들
	RIO_CQ rioCq_{ RIO_INVALID_CQ };

	// Completion 알림
	RIO_NOTIFICATION_COMPLETION completionType_{};

};

