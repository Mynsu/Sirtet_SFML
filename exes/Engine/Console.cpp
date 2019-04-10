#pragma hdrstop

#include "Console.h"
#include <iostream>//

class ConsoleLocal : public IConsole
{
public:
	ConsoleLocal( )
	{
		//TODO:
		std::cout << "Is ConsoleLocal instance really dynamically bind?\n";
	}
	~ConsoleLocal( ) = default;

	virtual void addCommand( const char* command,
							 std::function< void( void ) > function,
							 CommandType type,
							 const char* description )
	{

	}
};

std::unique_ptr< IConsole > Console = std::make_unique< ConsoleLocal >( );