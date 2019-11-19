//TODO: 솔트할까?

#pragma once
#include <stdint.h>

using HashedKey = uint32_t; //궁금: 어떻게 uint32_t를 알지?
using Dword = int32_t;

namespace util::hash
{
	constexpr uint8_t Measure( const char* str )
	{
		return ('\0'==*str)? 0u: 1+Measure( ++str );
	}

	constexpr HashedKey Digest( const char* arg, const uint8_t len )
	{
		HashedKey retHash = 0u;
		for ( uint8_t i = 0u; i != len; ++i )
		{
			retHash += 65599*retHash + arg[i];
		}
		return retHash ^ (retHash>>16);
	}

	constexpr HashedKey Digest( uint32_t arg )
	{
		HashedKey retHash = 0u;
		while ( 0 != arg )
		{
			retHash += 65599*retHash + 48 + arg%10;
			arg /= 10;
		}
		return retHash ^ (retHash>>16);
	}


}