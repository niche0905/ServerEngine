#pragma once
#include "pch.h"

/*-------------
   IoObject
-------------*/
//
// IoObject는 비동기 입출력의 주체입니다
// 인터페이스 형태로 GetHandle()과 Dipatch()를 제공해야 합니다
//

class IoObject : public std::enable_shared_from_this<IoObject>
{
public:
	virtual HANDLE GetHandle() = 0;
	virtual void Dispatch(class IIoEvent* ioEvent, int32 numOfBytes = 0) = 0;

};

