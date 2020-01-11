#pragma once
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32")
#include <stdint.h>

class EndPoint
{
public:
	EndPoint( );
	EndPoint( const char* const ipAddress, const uint16_t port );
	inline const SOCKADDR_IN& get( ) const
	{
		return mIPv4EP;
	}

	static EndPoint Any;
private:
	SOCKADDR_IN mIPv4EP;
};