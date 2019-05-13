#pragma once
#pragma hdrstop
#include <unordered_map>
#include <memory>
#include <intrin.h> // __debugbreak( ), int32_t
#include <SFML/System.hpp>
#include "Console.h"

using hashValue_t = int32_t;
using dword = int32_t;

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
	static auto VariableTable( ) -> std::unordered_map< hashValue_t, dword >&
	{
		return variableTable;
	}
private:
	static std::unique_ptr< IConsole > _Console;
	static std::unordered_map< hashValue_t, dword > variableTable;
};	