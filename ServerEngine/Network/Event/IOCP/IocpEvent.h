#pragma once
#include "Network/Event/IIoEvent.h"

/*-------------
   IocpEvent
-------------*/
//
// IocpEvent는 I/O Completion Port에서 사용하는 이벤트 객체입니다.
//

class IocpEvent : public OVERLAPPED, public IIoEvent
{
public:
	IocpEvent(IoEventType type);
	virtual ~IocpEvent() = default;

	void ResetOverlapped() noexcept
	{
		// ZeroMemory(static_cast<OVERLAPPED*>(this), sizeof(OVERLAPPED));
		
		OVERLAPPED::hEvent = 0;
		OVERLAPPED::Internal = 0;
		OVERLAPPED::InternalHigh = 0;
		OVERLAPPED::Offset = 0;
		OVERLAPPED::OffsetHigh = 0;
	}

	virtual IoEventType GetType() const noexcept override { return type_; }

	virtual void SetOwner(std::shared_ptr<IoObject> owner) override { owner_ = owner; }
	virtual std::shared_ptr<IoObject> GetOwner() const override { return owner_; }
	
	virtual void* GetNativeContext() noexcept override { return this; }

private:
	IoEventType					type_{ IoEventType::None };
	std::shared_ptr<IoObject>	owner_{ nullptr };

};