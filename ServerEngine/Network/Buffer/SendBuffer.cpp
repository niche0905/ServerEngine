#include "pch.h"
#include "SendBuffer.h"

/*--------------
   SendBuffer
--------------*/

bool SendBuffer::OnWrite(byte* data, size_t nums)
{
	if (data == nullptr || nums == 0)
		return false;

	if (writeSize_ + nums > buffer_.size())
		return false;

	memcpy(buffer_.data() + writeSize_, data, nums);
	writeSize_ += nums;
	return true;
}

void SendBuffer::OnRead(size_t nums)
{

}

void SendBuffer::Close(size_t nums)
{
	writeSize_ = nums;
}
