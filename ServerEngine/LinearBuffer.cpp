#include "pch.h"
#include "LinearBuffer.h"

LinearBuffer::LinearBuffer(size_t bufferSize)
	: RecvBuffer{ bufferSize }
{
	buffer_.resize(capacity_);
}

LinearBuffer::~LinearBuffer()
{

}

bool LinearBuffer::OnWrite(byte* data, size_t nums)
{
	if (nums > FreeSize())
		return false;

	std::memcpy(buffer_.data() + size_, data, nums);
	writePos_ += static_cast<int32>(nums);
	size_ += nums;
	return true;
}

void LinearBuffer::OnRead(size_t nums)
{
	if (nums >= Size())
		return;		// 잘못된 요청

	readPos_ += static_cast<int32>(nums);
	size_ -= nums;
	Clean();
	return;
}

DWORD LinearBuffer::PrepareRecv(WSABUF(&wsabuf)[2]) noexcept
{
	const size_t freeSize = FreeSize();

	wsabuf[0].buf = reinterpret_cast<char*>(buffer_.data() + writePos_);
	wsabuf[0].len = static_cast<ULONG>(freeSize);

	return 1;
}

void LinearBuffer::Commit(size_t nums) noexcept
{
	if (nums > FreeSize())
		return;		// 잘못된 요청

	writePos_ += static_cast<int32>(nums);
	size_ += nums;
	return;
}

std::pair<const byte*, size_t> LinearBuffer::Peek() const noexcept
{
	const size_t dataSize = DataSize();

	if (dataSize > 0)
		return std::pair<const byte*, size_t>(buffer_.data() + readPos_, dataSize);

	return std::pair<const byte*, size_t>();
}

void LinearBuffer::Consume(size_t nums) noexcept
{
	OnRead(nums);
}

void LinearBuffer::Clean()
{
	const size_t dataSize = DataSize();

	if (dataSize == 0) {
		if (size_ != 0) {
			// 문제 상황
			assert(false);
		}

		writePos_ = 0;
		readPos_ = 0;
	}
	else {
		// 넉넉하게 잡은 버퍼가 거의 다 찼을 때
		if (FreeSize() < bufferSize_) {
			MoveData();
		}
	}
}

void LinearBuffer::MoveData()
{
	const size_t dataSize = DataSize();

	std::memcpy(buffer_.data(), buffer_.data() + readPos_, dataSize);
	readPos_ = 0;
	writePos_ = static_cast<int32>(dataSize);
}
