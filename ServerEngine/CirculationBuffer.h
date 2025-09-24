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
	std::pair<const byte*, size_t>		Peek() const noexcept override;
	void								Consume(size_t nums) noexcept override;
	size_t								DataSize() const noexcept override { return size_; }
	size_t								FreeSize() const noexcept override { return HasWrapped()? ContinuousSize() : ContinuousSize() + SpareSize(); }

private:
	bool								HasWrapped() const noexcept { return writePos_ < readPos_; }

	size_t								ContinuousSize() const noexcept { return HasWrapped() ? (readPos_ - writePos_) : (capacity_ - writePos_); }
	size_t								SpareSize() const noexcept { return HasWrapped() ? (readPos_ - writePos_) : readPos_ ; }

	void								Clean();

private:
	int32								writePos_ = 0;
	int32								readPos_ = 0;
	std::vector<byte>					buffer_;

};

