#pragma once
#include <memory>

class IConsole
{
public:
	IConsole( ) = default;
	virtual ~IConsole( ) = default;
};

extern std::unique_ptr< IConsole > Console;