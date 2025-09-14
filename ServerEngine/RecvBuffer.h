#pragma once
#include "BaseBuffer.h"

/*--------------
   RecvBuffer
--------------*/
//
// RecvBuffer는 BYTE형식의 데이터를 담는 그릇입니다
// RecvBuffer는 데이터를 받기 위한 용도로 사용됩니다
// 

class RecvBuffer : public BaseBuffer<byte>
{
protected:
	inline static constexpr size_t BUFFER_COUNT = 10;

public:
	// 호출자가 제공하는 WSABUF[2]를 채워줍니다
	// 반환값은 WSABUF에 채워진 개수입니다 (0 ~ 2)
	virtual DWORD PrepareRecv(WSABUF (&wsabuf)[2]) noexcept abstract;
	
	// WSABUF에서 사용된 만큼 데이터를 버퍼에 커밋합니다
	virtual void Commit(size_t nums) noexcept abstract;

	// 파서가 버퍼에 담긴 데이터를 제거하지 않고 읽기만 합니다
	virtual std::pair<const byte*, size_t> Peek() const noexcept abstract;

	// 파서가 버퍼에 담긴 데이터를 제거합니다
	virtual void Consume(size_t nums) noexcept abstract;

	// 버퍼에 남은 여유 공간의 크기를 반환합니다
	virtual size_t FreeSize() const noexcept abstract;

};
