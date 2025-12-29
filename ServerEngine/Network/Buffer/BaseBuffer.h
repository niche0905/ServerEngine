#pragma once
#include "pch.h"

/*--------------
   BaseBuffer
--------------*/
//
// BaseBuffer는 데이터를 담는 그릇입니다
// Data는 실제 데이터를 담는 포인터입니다 (시작 주소를 가리킴)
// Size는 현재 담긴 데이터의 크기입니다
// Capacity는 할당된 데이터의 최대 크기입니다
//

template<typename T>
class BaseBuffer : public std::enable_shared_from_this<BaseBuffer<T>>
{
public:
	virtual T*			Data() = 0;
	virtual const T*	Data() const = 0;
	virtual size_t		Size() const = 0;
	virtual size_t		Capacity() const = 0;

	virtual bool		OnWrite(T* data, size_t nums) = 0;		// 데이터를 버퍼에 씁니다
	virtual void		OnRead(size_t nums) = 0;				// 데이터를 버퍼에서 읽습니다 (읽은 만큼 버퍼에서 제거)	
	
};

