#pragma once
#include <stdint.h>

using HashedKey = uint32_t;
using Dword = int32_t;

namespace util::hash
{
	// 문자열의 길이를 반환합니다.
	// NOTE: 런타임에는 ::strlen(...)이 더 빠릅니다.
	constexpr uint8_t Measure( const char* str )
	{
		return ('\0'==*str)? 0: 1+Measure( ++str );
	}

	////
	// !IMPORTANT: Digest(...)와 Digest2(...)는 입력이 같으면 출력도 같아야 함.
	///

	// 65599 문자열 해싱 함수입니다.
	template <int N>
	constexpr HashedKey Digest( const char (&str)[N] )
	{
		HashedKey retHash = 0;
		for ( uint8_t i = 0; N-1 != i; ++i )
		{
			retHash = str[i] + (retHash<<6) + (retHash<<16) - retHash;
		}
		return retHash & 0x7fffffff;
	}

	// 65599 문자열 해싱 함수입니다.
	inline HashedKey Digest2( const std::string& str )
	{
		HashedKey retHash = 0;
		const char* ptr = str.data();
		while ( *ptr )
		{
			retHash = *ptr++ + (retHash<<6) + (retHash<<16) - retHash;
		}
		return retHash & 0x7fffffff;
	}
}