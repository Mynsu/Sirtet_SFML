#pragma hdrstop
#include "Endian.h"

namespace util
{
	namespace endian
	{
		Dword( *ConvertToLittle )( Dword arg );
		Dword( *ConvertToBig )( Dword arg );

		Dword _Convert( Dword arg )
		{
			using Size = char;
			Size numShift = static_cast< Size >( sizeof( Dword ) * 8u - 8u );
			Dword mask = 0xff;
			Byte temp = arg & mask;
			arg |= ( arg >> numShift ) & mask;
			arg |= temp << numShift;
			mask <<= 8;
			temp = arg & mask;
			numShift -= 16u;
			arg |= ( arg >> numShift ) & mask;
			arg |= temp << numShift;

			return arg;
		}

		Dword _NoAction( Dword arg )
		{
			return arg;
		}

		void BindConvertFunc( )
		{
			union
			{
				Byte half;
				Word whole;
			} test;

			test.whole = 1;
			// When now on LITTLE endian,
			if ( 1 == test.half )
			{
				ConvertToBig = _Convert;
				ConvertToLittle = _NoAction;
			}
			// When now on BIG endian,
			else
			{
				ConvertToBig = _NoAction;
				ConvertToLittle = _Convert;
			}
		}
	}

	//TODO: 오브젝트 풀 구현, 다른 파일에서.
	namespace memory
	{
		//bool OccupyMemPool( byte** ptr, size_t size )
		//{
		//	bool retVal = true;

		//	//byte** _ptr = ( byte** )ptr;
		//	//*_ptr = new byte[ size ];

		//	*ptr = new byte[ size ];
		//	for ( size_t i = 0; i != size; ++i )
		//	{
		//		//*_ptr[ i ] = 0;
		//		*ptr[ i ] = 0;
		//	}

		//	if ( nullptr == ptr )
		//	{
		//		retVal = false;
		//	}

		//	return retVal;
		//}

		//void SetFreeMemPool( byte** ptr )
		//{
		//	delete[ ] *ptr;
		//	*ptr = nullptr;
		//}
	}
}