#pragma once

/*-----------
   IoCore
-----------*/
//
// IoCore로 통신 구조를 담당하는 Handle과 관련된 것들을 담당하는 class입니다
// 이 class를 상속하여 IocpCore와 RioCore를 구성할 것입니다
// 공통된 필수 함수를 가상함수로 설정하였습니다
//

class Session;

class IoCore
{
public:
	IoCore();
	~IoCore();

	bool Initialize();

	bool Dispatch(DWORD timeoutMs = INFINITE);
	
	void AttachSession(Session session);
	void DetachSession(Session session);

private:


};

