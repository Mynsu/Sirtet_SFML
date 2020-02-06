#pragma once
#include "Command.h"

class Console : public IConsole
{
public:
	Console( );
	Console( const Console& ) = delete;
	void operator=( const Console& ) = delete;
	Console( Console&& ) = delete;
	virtual ~Console( ) = default;
	
	void print( const std::string& message, const sf::Color color = sf::Color::White ) override;
	void printFailure( const FailureLevel failureLevel, const std::string& message ) override;
	void printScriptError( const ExceptionType exceptionType, const std::string& variableName, const std::string& scriptName ) override;
	void addCommand( const HashedKey command, const Func& functional ) override;
	void removeCommand( const HashedKey command ) override;
	bool isVisible( ) const override;
	
	void draw( sf::RenderWindow& window );
	void handleEvent( std::vector<sf::Event>& eventQueue );
	void processCommand( const std::string& commandLine )
	{
		mCommand.processCommand( commandLine );
	}
	void release( )
	{
		mCommand.release( );
	}
	void initialize( );
private:
	void refresh( const std::string_view& )
	{
		initialize( );
	}
	bool mVisible;
	uint32_t mFontSize, mLinesShown;
	std::string mCurrentInput;
	Command mCommand;
	sf::Font mFont;
	sf::RectangleShape mConsoleBackground;
	std::array<std::string, (int)ExceptionType::_MAX> mExceptionTypes;
	sf::Text mCurrentInputTextField, mHistoryTextLabel;
	std::vector<std::pair<std::string, sf::Color>> mHistory;
};