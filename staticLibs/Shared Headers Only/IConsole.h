////
//  Shared by exes/Engine, dlls/Game.
////

#pragma once
#include <functional>
#include <string>
#include <string_view>
#include <SFML/Graphics.hpp>
#include "../Lib/Hash.h"

using Func = std::function<void(const std::string_view& args)>;

enum class FailureLevel
{
	// When the default value is used as a makeshift and using an explicit value is strongly recommended
	WARNING,
	// When it's obvious that a crush or a severe failure will happen
	FATAL,
};

enum class ExceptionType
{
	VARIABLE_NOT_FOUND,
	TYPE_CHECK,
	FILE_NOT_FOUND,
	RANGE_CHECK,

	_MAX,
};

class IConsole
{
public:
	virtual ~IConsole( ) = default;

	virtual void print( const std::string& message, const sf::Color color = sf::Color::White ) = 0;
	virtual void printFailure( const FailureLevel failureLevel, const std::string& message ) = 0;
	// Not includes the case that a script file itself can't be found.
	virtual void printScriptError( const ExceptionType exceptionType,
								  const std::string& variableName,
								  const std::string& scriptName ) = 0;
	virtual void addCommand( const HashedKey command, const Func& functional ) = 0;
	virtual void removeCommand( const HashedKey command ) = 0;
	virtual bool isVisible( ) const = 0;
protected:
	IConsole( ) = default;
};