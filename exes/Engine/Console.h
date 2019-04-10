#pragma once
#include <memory>
#include <functional>

class IConsole
{
public:
	enum class CommandType
	{
		SYSTEM,
		UNKNOWN,
	};

public:
	IConsole( ) = default;
	virtual ~IConsole( ) = default;

	virtual void addCommand( const char* command,
							 std::function< void( void ) > function,
							 CommandType type,
							 const char* description ) = 0;
};

extern std::unique_ptr< IConsole > Console;