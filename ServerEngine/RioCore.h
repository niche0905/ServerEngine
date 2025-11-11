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
	void DetachIoObject(std::shared_ptr<IoObject> ioObject) override;

private:
	// TODO: RIO 관련 멤버 변수들 추가
	// TODO: RIO 관련 멤버 변수 추가에 따른 메서드 구현

};

