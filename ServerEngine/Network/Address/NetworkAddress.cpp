#include "pch.h"
#include "NetworkAddress.h"

/*------------------
   NetworkAddress
------------------*/

NetworkAddress::NetworkAddress(SOCKADDR_IN sockAddr)
	: sockAddr_(sockAddr)
{

}

NetworkAddress::NetworkAddress(std::wstring ip, uint16 port)
{
	::memset(&sockAddr_, 0, sizeof(sockAddr_));
	sockAddr_.sin_family = AF_INET;
	sockAddr_.sin_addr = Ip2Address(ip.c_str());
	sockAddr_.sin_port = ::htons(port);
}

std::wstring NetworkAddress::GetIpAddress() const
{
	WCHAR ip[INET_ADDRSTRLEN] = {};
	if (::InetNtopW(AF_INET, (void*)&sockAddr_.sin_addr, ip, INET_ADDRSTRLEN))
	{
		return std::wstring(ip);
	}
	return std::wstring();
}

IN_ADDR NetworkAddress::Ip2Address(const WCHAR* ip)
{
	IN_ADDR addr{};
	::InetPtonW(AF_INET, ip, &addr);
	return addr;
}
