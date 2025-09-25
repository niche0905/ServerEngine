#pragma once
#include "BaseBuffer.h"
#include "ReadView.h"

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
	RecvBuffer() : capacity_(0), size_(0), bufferSize_(0) {}
	virtual ~RecvBuffer() {}

	RecvBuffer(size_t bufferSize) : bufferSize_(static_cast<int32>(bufferSize)), capacity_(bufferSize * BUFFER_COUNT), size_(0) {}

public:
	size_t										Size() const override { return size_; }
	size_t										Capacity() const override { return capacity_; }

public:
	// 호출자가 제공하는 WSABUF[2]를 채워줍니다
	// 반환값은 WSABUF에 채워진 개수입니다 (0 ~ 2)
	virtual DWORD								PrepareRecv(WSABUF (&wsabuf)[2]) noexcept = 0;
	
	// WSABUF에서 사용된 만큼 데이터를 버퍼에 커밋합니다
	virtual void								Commit(size_t nums) noexcept = 0;

	// 파서가 버퍼에 담긴 데이터를 제거하지 않고 읽기만 합니다
	virtual ReadView							PeekView() const noexcept = 0;

	// 파서가 버퍼에 담긴 데이터를 제거하지 않고 읽기만 합니다(최대 1개 구간 <= 단일 구간)
	virtual std::pair<const byte*, size_t>		Peek() const noexcept
	{
		// 하위 호환: 첫 연속 구간만 노출
		ReadView v = PeekView();
		return { v.seg1.buffer, v.seg1.length };
	}

	// 파서가 버퍼에 담긴 데이터를 제거하지 않고 읽기만 합니다(need 크기 만큼 읽기 시도, 불가능하면 false) dst에 복사
	virtual bool								PeekInto(void* dst, size_t need) const noexcept = 0;

	virtual size_t								PeekAll(void* dst) const noexcept
	{
		PeekInto(dst, size_);
		return size_;
	}

	// 파서가 버퍼에 담긴 데이터를 제거합니다
	virtual void								Consume(size_t nums) noexcept = 0;

	// 버퍼에 담긴 데이터의 크기를 반환합니다
	virtual size_t								DataSize() const noexcept { return size_; }

	// 버퍼에 남은 여유 공간의 크기를 반환합니다
	virtual size_t								FreeSize() const noexcept { return (capacity_ - size_); }

protected:
	int32 capacity_;
	int32 size_;
	int32 bufferSize_;

};
