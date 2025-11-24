#pragma once

/*-----------------
   ThreadManager
-----------------*/
//
// ThreadManager는 thread 관리를 담당합니다
//

class ThreadManager
{
public:
	ThreadManager();
	~ThreadManager();

public:
	void Launch(std::function<void()> func);
	void Join();

	static void DoReserveWork();
	static void DoGlobalWork();

private:
	static void InitTLS();
	static void DestroyTLS();

private:
	std::vector<std::thread>	threads_;	// 관리하는 thread 들
	std::mutex					mutex_;		// thread 안전을 위한 mutex

};

