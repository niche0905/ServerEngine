#include "pch.h"
#include "PacketSession.h"

/*-----------------
   PacketSession
-----------------*/

int32 PacketSession::OnRecv(byte* buffer, int32 len)
{
	int32 processLen = 0;

	while (true)
	{
		int32 dataSize = len - processLen;
		
		// PacketHeader over
		if (dataSize < GetPacketHeaderSize())
			break;

		// Full Packet over
		int32 packetSize = GetPacketSize(buffer + processLen);
		if (dataSize < packetSize)
			break;

		OnRecvPacket(buffer + processLen, packetSize);

		processLen += packetSize;
	}

	return processLen;
}
