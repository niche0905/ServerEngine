#include "pch.h"
#include "CirculationBuffer.h"

CirculationBuffer::CirculationBuffer(size_t bufferSize)
	: RecvBuffer{ bufferSize }
{
	buffer_.resize(capacity_);
}

CirculationBuffer::~CirculationBuffer()
{

}

bool CirculationBuffer::OnWrite(byte* data, size_t nums)
{
	if (nums > FreeSize())
		return false;

	const size_t continousSize = ContinuousSize();
	const size_t spareSize = SpareSize();

	size_t remainSize = nums;

	if (remainSize > continousSize) {
		std::memcpy(buffer_.data() + writePos_, data, continousSize);
		remainSize -= continousSize;
		data += continousSize;
		writePos_ = 0;

		std::memcpy(buffer_.data(), data, remainSize);
		writePos_ += static_cast<int32>(remainSize);
		size_ += nums;
	}
	else {
		std::memcpy(buffer_.data() + writePos_, data, remainSize);
		writePos_ += static_cast<int32>(remainSize);
		size_ += nums;
	}

	return true;
}

void CirculationBuffer::OnRead(size_t nums)
{
	if (nums >= Size())
		return;		// 잘못된 요청

	readPos_ += static_cast<int32>(nums);
	// 환형 버퍼이므로 readPos_가 capacity_를 넘으면 readPos_를 0부터 다시 시작
	readPos_ = (readPos_ >= capacity_) ? readPos_ - capacity_ : readPos_;
	size_ -= nums;
	Clean();
	return;
}

DWORD CirculationBuffer::PrepareRecv(WSABUF(&wsabuf)[2]) noexcept
{
	bool wrapped = HasWrapped();
	const size_t continousSize = ContinuousSize();
	const size_t spareSize = SpareSize();

	if (continousSize == 0 && spareSize == 0)
		// 문제가 되는 상황
		return 0;

	if (wrapped) {
		// writePos_가 readPos_보다 앞에 있는 상황
		wsabuf[0].buf = reinterpret_cast<char*>(buffer_.data() + writePos_);
		wsabuf[0].len = static_cast<ULONG>(continousSize);
		return 1;
	}
	else {

		wsabuf[0].buf = reinterpret_cast<char*>(buffer_.data() + writePos_);
		wsabuf[0].len = static_cast<ULONG>(continousSize);

		if (spareSize == 0)
			return 1;

		wsabuf[1].buf = reinterpret_cast<char*>(buffer_.data());
		wsabuf[1].len = static_cast<ULONG>(spareSize);

		if (continousSize == 0)
			return 1;

		return 2;
	}

}

void CirculationBuffer::Commit(size_t nums) noexcept
{
	if (nums > FreeSize())
		return;		// 잘못된 요청

	writePos_ += static_cast<int32>(nums);
	size_ += nums;

	// 환형 버퍼이므로 writePos_가 capacity_를 넘으면 writePos_를 0부터 다시 시작
	writePos_ = (writePos_ >= capacity_) ? writePos_ - capacity_ : writePos_;

	return;
}

std::pair<const byte*, size_t> CirculationBuffer::Peek() const noexcept
{
	const size_t dataSize = DataSize();

	// TODO: 환형 버퍼이므로 readPos_가 capacity_를 넘으면 readPos_를 0부터 다시 시작 (해당 배열의 안정성을 보장해 주어야 한다)
	if (dataSize > 0)
		return std::pair<const byte*, size_t>(buffer_.data() + readPos_, dataSize);

	return std::pair<const byte*, size_t>();
}

void CirculationBuffer::Consume(size_t nums) noexcept
{
	OnRead(nums);
}

void CirculationBuffer::Clean()
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
}
