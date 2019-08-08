//TODO: 솔트할까?

#pragma once
#include <stdint.h> // uint32_t

using HashedKey = uint32_t;

namespace util::hash
{
	constexpr HashedKey Digest( const char arg[ ] )
	{
		constexpr unsigned short len = sizeof( arg ) / sizeof( *arg ); //궁금: 왜 8로 정해지지?
		HashedKey retHash = 0;
		for ( unsigned short i = 0; i != len; ++i )
		{
			retHash += 65599 * retHash + arg[ i ];
		}

		return retHash ^ ( retHash >> 16 );
	}
}