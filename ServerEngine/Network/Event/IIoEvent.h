#pragma once
#include "pch.h"

class IoObject;

enum class IoEventType : uint8
{
	None,			// default

	Connect,		// Connect는 비동기 연결의 결과를 나타냅니다
	Disconnect,		// Disconnect는 비동기 연결 해제의 결과를 나타냅니다
	Accept,			// Accept는 비동기 수신 연결의 결과를 나타냅니다
	Recv,			// Recv는 비동기 수신의 결과를 나타냅니다
	Send,			// Send는 비동기 송신의 결과를 나타냅니다

	End

};

/*------------
   IIoEvent
------------*/
//
// IIoEvent는 비동기 입출력 이벤트의 인터페이스입니다
//

class IIoEvent 
{
public:
	virtual ~IIoEvent() = default;

	virtual IoEventType GetType() const noexcept = 0;

	virtual void SetOwner(std::shared_ptr<IoObject> owner) = 0;
	virtual std::shared_ptr<IoObject> GetOwner() const = 0;

	virtual void* GetNativeContext() noexcept = 0;

};