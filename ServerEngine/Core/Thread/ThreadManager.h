#pragma once

/*-----------------
   ThreadManager
-----------------*/
//
// ThreadManager는 thread 관리를 담당합니다
// 
// 사용 예시
// 전역 변수로 접근
// GThreadManager.Launch([]()
// {
//    do something...
// });
//

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

public:
	// 새로운 thread 생성 및 실행
	void Launch(std::function<void()> func);
	// 모든 thread 종료 대기
	void Join();

	// 각자 thread에서 처리할 작업
	static void DoReserveWork();
	// 모든 thread에서 공통으로 처리할 작업
	static void DoGlobalWork();

private:
	// thread 별로 TLS 초기화 및 종료
	static void InitTLS();
	static void DestroyTLS();

private:
	std::vector<std::thread>	threads_;	// 관리하는 thread 들
	std::mutex					mutex_;		// thread 안전을 위한 mutex

};

