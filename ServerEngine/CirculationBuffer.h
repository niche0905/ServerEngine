#pragma once
#include "RecvBuffer.h"

/*---------------------
   CirculationBuffer
---------------------*/
//
// CirculationBuffer는 환형 버퍼입니다
// CirculationBuffer는 BaseBuffer를 상속받아 구현됩니다
// CirculationBuffer는 PrepareRecv에서 Scatter-Gather 기법을 사용하여, 
// 환형 버퍼의 끝과 처음에 이어서 데이터를 받을 수 있도록 합니다
// 

class CirculationBuffer : public RecvBuffer
{
public:
	CirculationBuffer(size_t bufferSize = 1024);
	~CirculationBuffer();

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
	size_t								DataSize() const noexcept override { return size_; }
	size_t								FreeSize() const noexcept override { return capacity_ - size_; }

private:
	bool								HasWrapped() const noexcept { return writePos_ < readPos_; }

	size_t								FreeSeg1() const noexcept { return HasWrapped() ? (readPos_ - writePos_) : (capacity_ - writePos_); }
	size_t								FreeSeg2() const noexcept { return HasWrapped() ? 0 : readPos_; }

	void								Clean();

private:
	size_t								writePos_ = 0;
	size_t								readPos_ = 0;
	std::vector<byte>					buffer_;

};

