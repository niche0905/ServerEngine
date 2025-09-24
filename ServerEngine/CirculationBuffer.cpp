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

	size_t remainSize = nums;

	// 1st segment
	const size_t seg1 = FreeSeg1();
	const size_t moveSize = (remainSize <= seg1) ? remainSize : seg1;
	std::memcpy(buffer_.data() + writePos_, data, moveSize);
	writePos_ = static_cast<int32>((writePos_ + moveSize) % capacity_);
	size_ += moveSize;
	remainSize -= moveSize;
	data += moveSize;

	// 2nd segment (non-wrap에서만 남음)
	if (remainSize > 0) {
		// seg2 == readPos_ (버퍼 처음부터)
		std::memcpy(buffer_.data(), data, remainSize);
		writePos_ = static_cast<int32>(remainSize); // 0에서 n만큼 이동
		size_ += remainSize;
	}

	return true;
}

void CirculationBuffer::OnRead(size_t nums)
{
	if (nums > Size())
		return;		// 잘못된 요청

	// 환형 버퍼이므로 readPos_가 capacity_를 넘으면 readPos_를 0부터 다시 시작
	readPos_ = static_cast<int32>((readPos_ + nums) % capacity_);
	size_ -= nums;
	Clean();
	return;
}

DWORD CirculationBuffer::PrepareRecv(WSABUF(&wsabuf)[2]) noexcept
{
	const size_t seg1 = FreeSeg1();
	const size_t seg2 = FreeSeg2();

	if (seg1 == 0 && seg2 == 0)
		// 문제가 되는 상황
		return 0;

	if (seg1 > 0) {
		wsabuf[0].buf = reinterpret_cast<char*>(buffer_.data() + writePos_);
		wsabuf[0].len = static_cast<ULONG>(seg1);
		if (seg2 > 0) {
			wsabuf[1].buf = reinterpret_cast<char*>(buffer_.data());
			wsabuf[1].len = static_cast<ULONG>(seg2);
			return 2;
		}
		return 1;
	}
	else {
		// seg1 == 0, seg2 > 0
		wsabuf[0].buf = reinterpret_cast<char*>(buffer_.data());
		wsabuf[0].len = static_cast<ULONG>(seg2);
		return 1;
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
	if (size_ == 0) {
		writePos_ = 0;
		readPos_ = 0;
	}
}
