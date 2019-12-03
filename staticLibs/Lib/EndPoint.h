#pragma once
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32")
#include <stdint.h>

class EndPoint
{
public:
	EndPoint( );
	EndPoint( char* const ipAddress, uint16_t port );
	inline SOCKADDR_IN get( ) const
	{
		return mIPv4EP;
	}

	static EndPoint Any;
private:
	SOCKADDR_IN mIPv4EP;
};

const uint16_t QUEUE_SERVER_PORT = 10000u;
const uint16_t MAIN_SERVER_PORT = 54321u;