#pragma once

namespace util::endian
{
	using byte = char;
	using word = unsigned short;
	using dword = int;

	extern dword ( *ConvertToLittle )( dword arg );
	extern dword ( *ConvertToBig )( dword arg );

	dword _Convert( dword arg );
	dword _NoAction( dword arg );
	//�ñ���: ���� ��?
	/*{
		return arg;
	}*/

	void BindConvertFunc( );
}