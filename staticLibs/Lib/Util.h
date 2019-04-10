#pragma once
#include "stdafx.h"

namespace util
{
	using byte = char;
	using word = unsigned short;
	using dword = int;

	namespace endian
	{
		extern dword ( *ConvertToLittle )( dword arg );
		extern dword ( *ConvertToBig )( dword arg );

		dword _Convert( dword arg );
		dword _NoAction( dword arg )
		{
			return arg;
		}

		void BindConvertFunc( );
	}

	namespace memory
	{
		// size unit is byte.
		bool OccupyMemPool( void** ptr, size_t size = 1024 * 1024 );
		void FreeMemPool( void** ptr );
	}
}