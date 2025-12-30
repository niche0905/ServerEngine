#include "pch.h"
#include "IocpServerService.h"
#include "IocpCore.h"
#include "PacketSession.h"

constexpr int SERVER_PORT = 8252;	// TEMP

int main()
{
	ServiceRef service = std::make_shared<IocpServerService>(
		NetAddr(L"127.0.0.1", SERVER_PORT),
		std::make_shared<PacketSession>,
		1000
	);
	
	assert(service->Start() == true);
}