#pragma once

/*------------------
   NetworkAddress
------------------*/
//
// NetworkAddress은 네트워크 주소를 나타냅니다
//

class NetworkAddress
{
public:
	NetworkAddress() = default;
	explicit NetworkAddress(SOCKADDR_IN sockAddr);
	explicit NetworkAddress(std::wstring ip, uint16 port);

	const SOCKADDR_IN& GetSockAddrIn() const { return sockAddr_; }
	std::wstring GetIpAddress() const;
	uint16 GetPort() const { return ::ntohs(sockAddr_.sin_port); }

public:
	static IN_ADDR Ip2Address(const WCHAR* ip);

private:
	SOCKADDR_IN sockAddr_{};

};

using NetAddr = NetworkAddress;
