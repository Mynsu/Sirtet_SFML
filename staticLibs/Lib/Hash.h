#pragma once
#include <stdint.h>

using HashedKey = uint32_t;
using Dword = int32_t;

namespace util::hash
{
	// ���ڿ��� ���̸� ��ȯ�մϴ�.
	// NOTE: ��Ÿ�ӿ��� ::strlen(...)�� �� �����ϴ�.
	constexpr uint8_t Measure( const char* str )
	{
		return ('\0'==*str)? 0: 1+Measure( ++str );
	}

	////
	// !IMPORTANT: Digest(...)�� Digest2(...)�� �Է��� ������ ��µ� ���ƾ� ��.
	///

	// 65599 ���ڿ� �ؽ� �Լ��Դϴ�.
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

	// 65599 ���ڿ� �ؽ� �Լ��Դϴ�.
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