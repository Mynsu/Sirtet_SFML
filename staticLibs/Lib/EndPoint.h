#pragma once
#include "Common.h"

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
