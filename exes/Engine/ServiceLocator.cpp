#include "ServiceLocator.h"

std::unique_ptr< IConsole > ServiceLocator::_Console
	= std::make_unique< ConsoleLocal >( );