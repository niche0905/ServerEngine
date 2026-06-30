#include "pch.h"
#include "ThreadManager.h"
#include "Core/Global/CoreGlobal.h"
#include "ThreadLocalStorage.h"
#include "Utils/Logger/ConsoleLogger.h"

/*-----------------
   ThreadManager
-----------------*/

ThreadManager::ThreadManager()
{
	// InitTLS();	// for main thread TLS
}

ThreadManager::~ThreadManager()
{
	Join();
}

void ThreadManager::Launch(std::function<void()> func)
{
	std::lock_guard<std::mutex> lock(mutex_);

	threads_.push_back(std::thread([=]()
		{
			InitTLS();
			func();
			DestroyTLS();
		}));
}

void ThreadManager::Join()
{
	for (std::thread& th : threads_) {
		if (th.joinable()) {

			th.join();
		}
	}

	threads_.clear();
}

void ThreadManager::InitTLS()
{
	static std::atomic<uint32> SThreadID{ 1 };
	TLS().thread_id = SThreadID.fetch_add(1);	// unique thread id assign
	
#ifdef USE_RIO
	if (TLS().rioBufferPool.Init(&SocketUtils::Rio, RioSendBlockSize, RioSendBlockCount) == false) {
		if (consoleLogger)
			consoleLogger->Log(Color::Yellow, L"[Thread] RIO send buffer pool init failed. threadId=%u blockSize=%u blockCount=%u\n", TLS().thread_id, RioSendBlockSize, RioSendBlockCount);
	}
#endif
}

void ThreadManager::DestroyTLS()
{
}

void ThreadManager::DoReserveWork()
{
	// TODO: reserve work 분배 (근데 샤딩 생각하면 분배가 아니라 각자 처리하는게 맞을지도?)
}

void ThreadManager::DoGlobalWork()
{
	// TODO: 공통된 작업 처리
}
