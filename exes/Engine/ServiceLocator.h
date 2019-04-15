#pragma once
#include "Console.h"

class ServiceLocator
{
//TODO:
//protected:
public:
	static IConsole& console( )
	{
		return *_Console;
	}
private:
	static IConsole* _Console;
};