//TODO: 솔트할까?

#pragma once
#include <stdint.h>

using HashedKey = uint32_t; //궁금: 어떻게 uint32_t를 알지?
using Dword = int32_t;

namespace util::hash
{
	constexpr uint8_t Measure( const char* str )
	{
		return ('\0'==*str)? 0: 1+Measure(++str);
	}

	constexpr HashedKey Digest( const char* arg, const uint8_t len )
	{
		HashedKey retHash = 0;
		for ( uint8_t i = 0; i != len; ++i )
		{
			retHash += 65599 * retHash + arg[ i ];
		}
		return retHash ^ ( retHash >> 16 );
	}
}