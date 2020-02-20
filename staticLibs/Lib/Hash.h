#pragma once
#include <stdint.h>

using HashedKey = uint32_t;
using Dword = int32_t;

namespace util::hash
{
	constexpr uint8_t Measure( const char* str )
	{
		return ('\0'==*str)? 0: 1+Measure( ++str );
	}

	template <int N>
	constexpr HashedKey Digest( const char (&str)[N] )
	{
		HashedKey retHash = 0;
		for ( uint8_t i = 0; N != i; ++i )
		{
			retHash = str[i] + (retHash<<6) + (retHash<<16) - retHash;
		}
		return retHash & 0x7fffffff;
	}

	inline HashedKey Digest2( const char* str )
	{
		HashedKey retHash = 0;
		while ( *str )
		{
			retHash = *str++ + (retHash<<6) + (retHash<<16) - retHash;
		}
		return retHash & 0x7fffffff;
	}
}