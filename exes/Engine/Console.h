#pragma once
#include "Command.h"

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

class IConsole : public sf::Drawable
{
public:
	virtual ~IConsole( ) = default;
	virtual void draw( sf::RenderTarget& target, sf::RenderStates states ) const = 0;
	
	virtual void handleEvent( std::list< sf::Event >& eventQueue ) = 0;
	virtual void print( const std::string& message, sf::Color color = sf::Color::White ) = 0;
	virtual void printFailure( const FailureLevel failureLevel, const std::string& message ) = 0;
	// Not includes the case that a script file itself can't be found.
	virtual void printScriptError( const ExceptionType exceptionType, const char* variableName, const char* scriptName ) = 0;
	virtual void addCommand( const HashedKey command, const Command::Func& functional ) = 0;
	virtual void processCommand( const std::string& command ) = 0;
	virtual bool isVisible( ) const = 0;
	// Tell how big the window size is, which decides the position and the size of the console.
	// 800*600 assumed by default.
	virtual void setPosition( const sf::Vector2u& winSize ) = 0;
	// NOTE: Protected constructor prevents users from instantiating the abstract class.
protected:
	IConsole( ) = default;
};

class ConsoleLocal final : public IConsole
{
public:
	ConsoleLocal( );
	~ConsoleLocal( ) = default;
	void draw( sf::RenderTarget& target, sf::RenderStates states ) const override;

	void handleEvent( std::list< sf::Event >& eventQueue ) override;
	void print( const std::string& message, sf::Color color = sf::Color::White ) override;
	void printFailure( const FailureLevel failureLevel, const std::string& message ) override;
	void printScriptError( const ExceptionType exceptionType, const char* variableName, const char* scriptName ) override;
	void addCommand( const HashedKey command, const Command::Func& functional ) override;
	void processCommand( const std::string& commandLine ) override;
	bool isVisible( ) const override;
	void setPosition( const sf::Vector2u& winSize ) override;
private:
	bool mVisible;
	Command mCommand;
	std::string mCurrentInput;
	sf::Font mFont;
	sf::RectangleShape mConsoleWindow;
	sf::Text mCurrentInputTextField;
	std::array< std::string, static_cast< size_t >( ExceptionType::_MAX )> mExceptionTypes;
	std::array< sf::Text, 9 > mHistoryTextFields;
};