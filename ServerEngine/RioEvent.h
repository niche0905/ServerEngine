#pragma once
#include "IIoEvent.h"

/*------------
   RioEvent
------------*/
//
// RioEvent는 Registered I/O에서 사용하는 이벤트 객체입니다.
//

class RioEvent : public IIoEvent
{
public:
	RioEvent(IoEventType type);
	virtual ~RioEvent() = default;

	virtual IoEventType GetType() const noexcept override { return type_; }
	virtual void SetOwner(std::shared_ptr<IoObject> owner) override { owner_ = owner; }
	virtual std::shared_ptr<IoObject> GetOwner() const override { return owner_; }

	void* GetNativeContext() noexcept override { return this; }

	RIO_BUF* GetRioBuffer() { return &rioBuffer_; }
	void SetResult(RIORESULT* result) { context_ = result; }

private:
	IoEventType					type_{ IoEventType::None };
	std::shared_ptr<IoObject>	owner_{ nullptr };
	RIO_BUF						rioBuffer_{};
	RIORESULT*					context_{ nullptr };

};

