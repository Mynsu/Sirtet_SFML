#pragma once
#pragma hdrstop
#include <memory>
#include <SFML/System.hpp>
#include "Console.h"

class ServiceLocator
{
public:
	static auto Console( ) -> std::unique_ptr< IConsole >&
	{
		return _Console;
	}
	static auto Console( const sf::Vector2u& winSize ) -> std::unique_ptr< IConsole >&
	{
		_Console->init( winSize );
		return _Console;
	}
private:
	static std::unique_ptr< IConsole > _Console;
};	