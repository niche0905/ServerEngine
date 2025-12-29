#pragma once
#include "Network/Session/IoObject.h"

/*-----------
   IoCore
-----------*/
//
// IoCore로 통신 구조를 담당하는 Handle과 관련된 것들을 담당하는 class입니다
// 이 class를 상속하여 IocpCore와 RioCore를 구성할 것입니다
// 공통된 필수 함수를 가상함수로 설정하였습니다
//

class IoObject;

class IoCore
{
public:
	IoCore();
	virtual ~IoCore();

	// IoBackend를 반환하는 순수 가상 함수 (IOCP 또는 RIO)
	virtual BackendType Backend() const noexcept = 0;

	// 초기화 및 종료를 위한 순수 가상 함수
	virtual bool Initialize() = 0;
	virtual void Terminate() = 0;

	// 이벤트 디스패치를 위한 순수 가상 함수 (IO 이벤트 처리)
	virtual bool Dispatch(DWORD timeoutMs = INFINITE) = 0;
	
	// 세션을 IoCore에 연결 및 해제하는 순수 가상 함수
	virtual bool AttachIoObject(std::shared_ptr<IoObject> ioObject) = 0;
	//virtual void DetachIoObject(std::shared_ptr<IoObject> ioObject) = 0;	// 등록된 핸들을 해제하는 건 없다더라

private:


};

