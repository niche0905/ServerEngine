#pragma once
#include "BaseBuffer.h"

/*--------------
   SendBuffer
--------------*/
//
// SendBuffer는 BYTE형식의 데이터를 담는 그릇입니다
// SendBuffer는 데이터를 보내기 위한 용도로 사용합니다
// 

class SendBuffer : public BaseBuffer<byte>, public std::enable_shared_from_this<SendBuffer>
{
public:
	SendBuffer() : buffer_(0) {}
	virtual ~SendBuffer() {}

	SendBuffer(size_t bufferSize) : buffer_(bufferSize) {}

	byte*			Data() override { return buffer_.data(); }
	const byte*		Data() const override { return buffer_.data(); }
	size_t			Size() const override { return writeSize_; }
	size_t			Capacity() const override { return buffer_.size(); }

	bool			OnWrite(byte* data, size_t nums) override;
	void			OnRead(size_t nums) override;

	void			Close(size_t nums);

private:
	std::vector<byte>			buffer_;
	size_t						writeSize_ = 0;			// 버퍼에 기록된 데이터 크기

};

