//TODO: ��Ʈ�ұ�?

#pragma once
#include <stdint.h> // uint32_t

using HashedKey = uint32_t;

namespace util::hash
{
	constexpr HashedKey Digest( const char arg[ ] )
	{
		constexpr unsigned short len = sizeof( arg ) / sizeof( *arg ); //�ñ�: �� 8�� ��������?
		HashedKey retHash = 0;
		for ( unsigned short i = 0; i != len; ++i )
		{
			retHash += 65599 * retHash + arg[ i ];
		}

		return retHash ^ ( retHash >> 16 );
	}
}