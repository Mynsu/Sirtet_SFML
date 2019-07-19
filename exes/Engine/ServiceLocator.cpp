#include "ServiceLocator.h"

std::unique_ptr< IConsole > ServiceLocator::_Console
	= std::make_unique< ConsoleLocal >( );

std::unordered_map< HashedKey, Dword > ServiceLocator::_VariableTable;