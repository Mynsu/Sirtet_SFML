#pragma once

namespace util::endian
{
	using Byte = char;
	using Word = unsigned short;
	using Dword = unsigned int; //int

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