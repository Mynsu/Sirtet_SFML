#include "ServiceLocator.h"
#include "Console.h"

IConsole* ServiceLocator::_Console = new ConsoleLocal( );