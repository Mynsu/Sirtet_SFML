#pragma hdrstop
#pragma once
#include <memory>
#include "Console.h"

class ServiceLocator
{
public:
	static auto Console( ) -> std::unique_ptr< IConsole >&
	{
		return _Console;
	}
private:
	static std::unique_ptr< IConsole > _Console;
};