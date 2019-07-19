//TODO: 솔트할까?

#pragma once
#include <stdint.h> // uint32_t

using HashedKey = uint32_t;

namespace util::hash
{
	constexpr HashedKey Digest( const char arg[ ] )
	{
		unsigned short len = sizeof( arg ) / sizeof( *arg );
		HashedKey retHash = 0;
		for ( unsigned short i = 0; i != len; ++i )
		{
			retHash += 65599 * retHash + arg[ i ];
		}

		return retHash ^ ( retHash >> 16 );
	}
}