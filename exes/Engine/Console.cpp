#pragma hdrstop
#include "Console.h"
//TODO:
#include <iostream>

ConsoleLocal::ConsoleLocal( )
{
	//TODO:
	std::cout << "Is ConsoleLocal instance really dynamically bind?\n";
}

void ConsoleLocal::addCommand( std::string_view command,
							   std::function<void( void )> function,
							   CommandType type,
							   std::string_view description )
{

}

void ConsoleLocal::draw( sf::RenderTarget& target, sf::RenderStates states ) const
{

}
