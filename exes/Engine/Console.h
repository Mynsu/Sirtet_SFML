#pragma once
#include "Command.h"

class Console final : public IConsole, public sf::Drawable
{
public:
	Console( );
	~Console( ) = default;
	
	void print( const std::string& message, sf::Color color = sf::Color::White ) override;
	void printFailure( const FailureLevel failureLevel, const std::string& message ) override;
	void printScriptError( const ExceptionType exceptionType, const char* variableName, const char* scriptName ) override;
	void addCommand( const HashedKey command, const Func& functional ) override;
	void removeCommand( const HashedKey command ) override;
	bool isVisible( ) const override;
	
	void draw( sf::RenderTarget& target, sf::RenderStates states ) const override;
	void handleEvent( std::list< sf::Event >& eventQueue );
	inline void processCommand( const std::string& commandLine )
	{
		mCommand.processCommand( commandLine );
	}
	inline void release( )
	{
		mCommand.release( );
	}
	// Tell how big the window size is, which decides the position and the size of the console.
	// 800*600 assumed by default.
	void setPosition( const sf::Vector2u& windowSize );
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