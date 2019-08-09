#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include <array>
#include "Command.h"

enum class FailureLevel
{
	// When the default value is used as a makeshift and using an explicit value is strongly recommended
	WARNING,
	// When it's obvious that a crush or a severe failure will happen
	CRITICAL, //TODO: FATAL ¾î¶§?
};

class IConsole : public sf::Drawable
{
public:
	virtual ~IConsole( ) = default;
	virtual void draw( sf::RenderTarget& target, sf::RenderStates states ) const = 0;
	
	virtual void handleEvent( const sf::Event& event ) = 0;
	virtual void print( const std::string& message, sf::Color color = sf::Color::White ) = 0;
	virtual void printFailure( const FailureLevel failureLevel, const std::string& message ) = 0;
	virtual void printScriptError( const FailureLevel failureLevel, const std::string& variableName, const std::string& scriptName ) = 0;
	virtual void addCommand( const HashedKey command, const Command::Func& functional ) = 0;
	virtual void processCommand( const std::string& command ) = 0;
	virtual bool isVisible( ) const = 0;
	// Tell how big the window size is, which decides the position and the size of the console.
	// 800*600 assumed by default.
	virtual void setPosition( const sf::Vector2u& winSize ) = 0;
protected:
	IConsole( ) = default;
};

class ConsoleLocal final : public IConsole
{
public:
	ConsoleLocal( );
	~ConsoleLocal( ) = default;
	void draw( sf::RenderTarget& target, sf::RenderStates states ) const override;

	void handleEvent( const sf::Event& event ) override;
	void print( const std::string& message, sf::Color color = sf::Color::White ) override;
	void printFailure( const FailureLevel failureLevel, const std::string& message ) override
	{
		// Concatenate error level info with message
		// NOTE: Using another timespace on memory is faster than in-place rearrangement in this situation.
		std::string concatenated;
		switch ( failureLevel )
		{
			case FailureLevel::WARNING:
				concatenated.assign( "WARNING: " + message );
				break;
			case FailureLevel::CRITICAL:
				concatenated.assign( "CRITICAL: " + message );
				break;
			default:
#ifdef _DEBUG
				__debugbreak( );
#else
				__assume( 0 );
				break;
#endif
		}
		print( concatenated, sf::Color::Red );
	}
	void printScriptError( const FailureLevel failureLevel, const std::string& variableName, const std::string& scriptName ) override
	{
		printFailure( failureLevel, "Variable '" + variableName + "' in " + scriptName + " can't be found or has an odd value." );
	}
	void addCommand( const HashedKey command, const Command::Func& functional )
	{
		mCommand.addCommand( command, functional );
	}
	void processCommand( const std::string& commandLine ) override
	{
		mCommand.processCommand( commandLine );
	}
	bool isVisible( ) const
	{
		return mVisible;
	}
	void setPosition( const sf::Vector2u& winSize ) override;
private:
	bool mVisible;
	Command mCommand;
	std::string mCurrentInput;
	///std::string mCursorForeground;
	sf::Font mFont;
	sf::RectangleShape mConsoleWindow;
	sf::Text mCurrentInputTextField;
	///sf::Text mCursorForegroundTextField;
	std::array< sf::Text, 9 > mHistoryTextFields;
};