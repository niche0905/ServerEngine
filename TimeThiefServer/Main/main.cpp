#include "pch.h"

#include <chrono>
#include <thread>
#include <vector>

#include "Core/Service/IOCP/IocpServerService.h"
#include "Core/IoCore/IocpCore/IocpCore.h"
#include "Core/Thread/ThreadManager.h"
#include "Network/Session/PlayerSession.h"
#include "Network/Session/SessionManager/SessionManager.h"

class PlayerSession;

constexpr int SERVER_PORT = 8252;	// TEMP


ThreadManager threadManager;

std::vector<std::string> testStrings = {
	"Hello, World!",
	"TimeThiefServer is running.",
	"This is a test packet.",
	"Networking with IOCP is powerful.",
	"Have a great day!"
};

void WorkerJob(ServiceRef& service)
{
	while (true) {

		service->Dispatch(1000);
	}
}

int main()
{
	consoleLogger->Log(Color::Green, L"[main] entered main\n");
	
	ServiceRef service = std::make_shared<IocpServerService>(
		NetAddr(L"127.0.0.1", SERVER_PORT),
		std::make_shared<PlayerSession>,
		5
	);

	consoleLogger->Log(Color::Green, L"[main] made service\n");
	
	// assert(service->Start() == true);
	bool ok = service->Start();
	if (!ok) {
		consoleLogger->Log(Color::Red, L"[main] Start FAIL\n");
		return 0;	
	}
	consoleLogger->Log(Color::Green, L"[main] Start OK\n");

	consoleLogger->Log(Color::Green, L"[main] before launching workers\n");
	int32 hc = (int32)std::thread::hardware_concurrency();
	int32 workerCount = (hc > 1) ? (hc - 1) : 1;
	for (int32 i = 0; i <  workerCount; ++i) {
		threadManager.Launch([&service]()
		{
			WorkerJob(service);
		});
	}
	
	consoleLogger->Log(Color::Green, L"[main] launched workers\n");

	int32 testIndex = 0;
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
		
		std::string message = testStrings[testIndex];
		testIndex = (testIndex + 1) % testStrings.size();
		
		std::shared_ptr<SendBuffer> sendBuffer = std::make_shared<SendBuffer>(message.size() + 1);
		
		uint8 size = static_cast<uint8>(message.size() + 1);
		sendBuffer->OnWrite(reinterpret_cast<byte*>(&size), 1);
		sendBuffer->OnWrite(reinterpret_cast<byte*>(message.data()), message.size());
		
		g_SessionManager.Broadcast(sendBuffer);
	}
	
	threadManager.Join();
}