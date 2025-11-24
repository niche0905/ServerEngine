#pragma once
#include "IIoEvent.h"

/*------------
   IIoEvent
------------*/
//
// IIoEvent는 비동기 입출력 이벤트의 인터페이스입니다
//

class IocpEvent : public IIoEvent, public OVERLAPPED
{
public:
	IocpEvent(IoEventType type);
	virtual ~IocpEvent() = default;

	virtual IoEventType GetType() const noexcept override { return type_; }

	virtual void SetOwner(std::shared_ptr<IoObject> owner) override { owner_ = owner; }
	virtual std::shared_ptr<IoObject> GetOwner() const override { return owner_; }
	
	virtual void* GetNativeContext() noexcept override { return this; }

private:
	IoEventType					type_{ IoEventType::None };
	std::shared_ptr<IoObject>	owner_{ nullptr };

};