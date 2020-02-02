#pragma once
#include <stdint.h>

namespace util::endian
{
	using Byte = uint8_t;
	using Word = uint16_t;
	using Dword = uint32_t;

	extern Dword ( *ConvertToLittle )( Dword arg );
	extern Dword ( *ConvertToBig )( Dword arg );

	Dword _Convert( Dword arg );
	Dword _NoAction( Dword arg );
	//�ñ���: ���� ��?
	/*{
		return arg;
	}*/

	void BindConvertFunc( );
}