#pragma once
#include "Network/Session/IoObject.h"
#include "Core/Service/ServiceBase.h"

/*------------
   Listener
------------*/
//
// Listener는 클라이언트의 접속을 수신하는 역할을 합니다
//

class Listener : public IoObject
{
public:
	Listener() = default;
	~Listener();

public:
	virtual HANDLE GetHandle() override;
	virtual void Dispatch(class IIoEvent* ioEvent, int32 numOfBytes = 0) override;

public:
	bool StartListening(std::shared_ptr<ServiceBase> service);
	void CloseListener();

private:
	void PostAccept(AcceptEvent* acceptEvent);
	void ProcessAccept(AcceptEvent* acceptEvent);

protected:
	SOCKET 							listenSocket_{ INVALID_SOCKET };
	std::vector<AcceptEvent*>		acceptEvents_;
	std::shared_ptr<ServiceBase>	service_{ nullptr };

};

