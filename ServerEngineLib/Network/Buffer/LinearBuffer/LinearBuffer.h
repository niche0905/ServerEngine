#pragma once
#include "Network/Buffer/RecvBuffer.h"

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
	LinearBuffer() = delete;
	LinearBuffer(size_t bufferSize = 1024);
	~LinearBuffer();

public:
	byte*								Data() override { return buffer_.data(); }
	const byte*							Data() const override { return buffer_.data(); }
	bool								OnWrite(byte* data, size_t nums) override;
	void								OnRead(size_t nums) override;

public:
	DWORD								PrepareRecv(WSABUF(&wsabuf)[2]) noexcept override;
	bool								Commit(size_t nums) noexcept override;
	ReadView							PeekView() const noexcept override;
	bool								PeekInto(void* dst, size_t need) const noexcept override;
	void								Consume(size_t nums) noexcept override;
	size_t								DataSize() const noexcept override { return size_; }
	size_t								FreeSize() const noexcept override { return (capacity_ - writePos_); }
	
public:
	size_t								WritePos() const noexcept { return writePos_; }
	size_t								ReadPos() const noexcept { return readPos_; }

private:
	void								Clean();
	void								MoveData();

private:
	size_t								writePos_ = 0;
	size_t								readPos_ = 0;
	std::vector<byte>					buffer_;

};

