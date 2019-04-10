#include "Util.h"

namespace util
{
	namespace endian
	{
		dword( *ConvertToLittle )( dword arg );
		dword( *ConvertToBig )( dword arg );

		dword _Convert( dword arg )
		{
			dword retVal = 0;
			unsigned int mask = 0xff;
			for ( int i = 1; i < 17; ++i )
			{
				retVal |= ( ( arg & mask ) << ( 32 - 2 * i ) );
				arg >> 2 * i;
			}

			return retVal;
		}

		void BindConvertFunc( )
		{
			union
			{
				byte half;
				word whole;
			} test;

			test.whole = 1;
			// When now on little endian,
			if ( 1 == test.half )
			{
				ConvertToBig = _Convert;
				ConvertToLittle = _NoAction;
			}
			else
			{
				ConvertToBig = _NoAction;
				ConvertToLittle = _Convert;
			}
		}
	}

	namespace memory
	{
		bool OccupyMemPool( void** ptr, size_t size )
		{
			bool retVal = true;
			byte** _ptr = ( byte** )ptr;
			*_ptr = new byte[ size ];
			for ( size_t i = 0; i != size; ++i )
			{
				*_ptr[ i ] = 0;
			}
			if ( nullptr == ptr )
			{
				retVal = false;
			}

			return retVal;
		}

		void FreeMemPool( void** ptr )
		{
			delete[ ] *ptr;
			*ptr = nullptr;
		}
	}
}