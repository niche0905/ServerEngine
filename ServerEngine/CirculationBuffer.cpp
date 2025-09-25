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

ReadView CirculationBuffer::PeekView() const noexcept
{
	ReadView view{};
	if (size_ == 0) return view;

	if (HasWrapped()) {
		// [read..end) 가 seg1, [0..write) 가 seg2
		view.seg1.buffer = buffer_.data() + readPos_;
		view.seg1.length = capacity_ - readPos_;
		view.seg2.buffer = buffer_.data();
		view.seg2.length = writePos_;
	}
	else {
		// [read..write)만 존재
		view.seg1.buffer = buffer_.data() + readPos_;
		view.seg1.length = writePos_ - readPos_;
		view.seg2.buffer = nullptr;
		view.seg2.length = 0;
	}

	return view;
}

bool CirculationBuffer::PeekInto(void* dst, size_t need) const noexcept
{
	if (need > size_) return false;
	auto v = PeekView();
	if (need <= v.seg1.length) {
		std::memcpy(dst, v.seg1.buffer, need);
		return true;
	}
	// 두 조각
	const size_t n1 = v.seg1.length;
	const size_t n2 = need - n1;
	std::memcpy(dst, v.seg1.buffer, n1);
	std::memcpy(static_cast<byte*>(dst) + n1, v.seg2.buffer, n2);
	return true;
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
