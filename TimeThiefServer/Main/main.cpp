#include "pch.h"

#include <chrono>
#include <thread>
#include <vector>

#include "Service/Room/RoomManager.h"
#include "Core/Service/IOCP/IocpServerService.h"
#include "Core/IoCore/IocpCore/IocpCore.h"
#include "Core/Thread/ThreadManager.h"
#include "Network/Session/PlayerSession.h"
#include "Network/Session/SessionManager/SessionManager.h"
#include "Network/ServerConfigReader.h"
#include "Generated/ServerPacketHandler.h"
#include "Service/MatchMaking/MatchMaker.h"

class PlayerSession;


ThreadManager threadManager;

void MatchingJob()
{
	while (true) {
		std::this_thread::sleep_for(std::chrono::milliseconds(200));
		
		g_MatchMaker.TryMatch();
	}
}

void WorkerJob(ServiceRef& service)
{
	while (true) {

		service->Dispatch(1000);
	}
}

int main()
{
	SE::Init();
	ServerPacketHandler::Init();
	g_RoomManager->CreateRoom();
	
	consoleLogger->Log(Color::Green, L"[main] entered main\n");
	
	bool configLoadSucc = g_ConfigReader.LoadFromFile("..\\External\\ProtocolShared\\config\\server.dev.json");
	if (not configLoadSucc) {
		consoleLogger->Log(Color::Red, L"[main] config load fail\n");
		return 0;
	}
	
	std::string serverIPStr = g_ConfigReader.Get().network.bindIp;
	std::wstring serverIP(serverIPStr.begin(), serverIPStr.end());
	int16 SERVER_PORT = g_ConfigReader.Get().network.gamePort;
	
	ServiceRef service = std::make_shared<IocpServerService>(
		NetAddr(serverIP, SERVER_PORT),
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
	int32 hc = static_cast<int32>(std::thread::hardware_concurrency());
	int32 workerCount = (hc > 1) ? (hc - 1) : 1;
	for (int32 i = 0; i <  workerCount; ++i) {
		threadManager.Launch([&service]()
		{
			WorkerJob(service);
		});
	}
	threadManager.Launch([]()
	{
		MatchingJob();
	});
	
	consoleLogger->Log(Color::Green, L"[main] launched workers\n");

	int32 testIndex = 0;
	while (true) {
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
	
	threadManager.Join();
}