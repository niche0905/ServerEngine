#pragma once
#include "SessionBase.h"

/*-----------------
   PacketSession
-----------------*/
//
// PacketSession는 패킷 기반 세션의 기본 클래스입니다
// Packet이 사용될 것을 전제로 PacketHeader를 처리하기 위해 함수가 제공됩니다
//

class PacketSession : public SessionBase
{
private:
#pragma pack(push, 1)
	struct DefaultPacketHeader
	{
		uint16 size;	// 패킷 전체 크기(헤더 포함)
		uint16 id;		// 패킷 아이디
	};
#pragma pack(pop)

	static_assert(sizeof(DefaultPacketHeader) == 4);

public:
	PacketSession() = default;
	virtual ~PacketSession() = default;

public:
	virtual std::shared_ptr<PacketSession> GetPacketSessionRef()
		{ return std::static_pointer_cast<PacketSession>(shared_from_this()); }

protected:
	virtual int32 GetPacketHeaderSize() const
	{
		return sizeof(DefaultPacketHeader);
	}
	virtual int32 GetPacketSize(byte* packet) const
	{
		DefaultPacketHeader* header = reinterpret_cast<DefaultPacketHeader*>(packet);
		return header->size;
	}

protected:
	virtual int32 OnRecv(byte* buffer, int32 len) override final;
	
	virtual bool CanPacketProcess(const byte* buffer, int32 len) = 0;
	virtual void OnRecvPacket(byte* buffer, int32 len) = 0;

};

