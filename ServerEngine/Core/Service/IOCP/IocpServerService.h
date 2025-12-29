#pragma once
#include "IocpService.h"
#include "Listener.h"

/*---------------------
   IocpServerService
---------------------*/
//
// IocpServerService는 IOCP 기반의 서버 서비스 인터페이스입니다
// Listener 세션을 생성하고 관리하는 기능을 포함합니다
//

class IocpServerService : public IocpService
{
public:
	IocpServerService() = delete;
	IocpServerService(NetAddr address, SessionFactory factory, int32 maxSessionCount = 1);
	virtual ~IocpServerService();

	// Service Start
	virtual bool Start() override;
	virtual void StopService() override;

private:
	std::shared_ptr<Listener> listener_{ nullptr };

};

