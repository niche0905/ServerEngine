#pragma once
#include "RecvBuffer.h"

/*----------------
   LinearBuffer
----------------*/
//
// LinearBuffer는 선형 버퍼입니다
// LinearBuffer는 BaseBuffer를 상속받아 구현됩니다
// LinearBuffer는 끝에 거의 다다랐을 때 앞쪽으로 데이터를 옮기는 방식으로 동작합니다
// 

class LinearBuffer : public RecvBuffer
{
public:
	LinearBuffer(size_t bufferSize = 1024);
	~LinearBuffer();

public:
	byte*								Data() override { return buffer_.data(); }
	bool								OnWrite(byte* data, size_t nums) override;
	void								OnRead(size_t nums) override;

public:
	DWORD								PrepareRecv(WSABUF(&wsabuf)[2]) noexcept override;
	void								Commit(size_t nums) noexcept override;
	ReadView							PeekView() const noexcept override;
	bool								PeekInto(void* dst, size_t need) const noexcept override;
	void								Consume(size_t nums) noexcept override;
	size_t								DataSize() const noexcept override { return (writePos_ - readPos_); }
	size_t								FreeSize() const noexcept override { return (capacity_ - writePos_); }

private:
	void								Clean();
	void								MoveData();

private:
	int32								writePos_ = 0;
	int32								readPos_ = 0;
	std::vector<byte>					buffer_;

};

