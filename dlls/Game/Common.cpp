#include "Common.h"

//IConsole* Console_ = nullptr;

namespace global
{
	std::unique_ptr< IConsole >& ( *Console )( ) = nullptr;
	std::unordered_map< hashValue_t, dword >& ( *VariableTable )( ) = nullptr;
}