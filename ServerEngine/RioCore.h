#pragma once
#include "IoCore.h"

/*------------
   RioCore
------------*/
//
// Windows의 Registered I/O (RIO) 방식을 사용하는 Core 입니다
//

class RioCore : public IoCore
{
public:
	IoBackend Backend() const noexcept override { return IoBackend::RIO; }
	
	bool Initialize() override;
	void Terminate() override;
	
	bool Dispatch(DWORD timeoutMs = INFINITE) override;
	
	bool AttachIoObject(std::shared_ptr<IoObject> ioObject) override;

public:
	// RIO 함수 테이블 접근자
	const RIO_EXTENSION_FUNCTION_TABLE& Rio() const noexcept { return rio_; }

	// CQ 핸들 접근자
	RIO_CQ GetCompletionQueue() const noexcept { return rioCq_; }

private:
	bool LoadRioFunctions();

private:
	// RIO 함수 테이블
	RIO_EXTENSION_FUNCTION_TABLE rio_{};

	// Completion Queue 핸들
	RIO_CQ rioCq_{ RIO_INVALID_CQ };

	// Completion Notify 핸들
	HANDLE completionEvent_{ INVALID_HANDLE_VALUE };

};

