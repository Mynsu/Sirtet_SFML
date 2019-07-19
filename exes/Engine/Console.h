#pragma once
#pragma hdrstop
#include <array>
#include <functional>
#include <string>
#include <SFML/Graphics.hpp>

enum class ErrorLevel
{
	// Use the default value instead
	WARNING,
	// Stop the machine
	CRITICAL,
};

class IConsole : public sf::Drawable
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
	virtual void init( const sf::Vector2u& winSize ) = 0;

	virtual void draw( sf::RenderTarget& target, sf::RenderStates states ) const = 0;
	
	virtual bool isVisible( ) const = 0;
	virtual void handleEvent( const sf::Event& event ) = 0;
	virtual void print( const std::string& message, sf::Color color = sf::Color::White ) = 0;
	virtual void printError( const ErrorLevel errorLevel, const std::string& errorMessage ) = 0;
	virtual void printScriptError( const ErrorLevel errorLevel, const std::string& variableName, const std::string& scriptName ) = 0;
	/*virtual void addCommand( const std::string& command,
							 std::function< void( void ) > function,
							 CommandType type,
							 const std::string& description ) = 0;*/
};

class ConsoleLocal final : public IConsole
{
public:
	ConsoleLocal( );
	~ConsoleLocal( ) = default;
	void init( const sf::Vector2u& winSize ) override;

	void draw( sf::RenderTarget& target, sf::RenderStates states ) const override;

	bool isVisible( ) const
	{
		return mVisible;
	}
	void handleEvent( const sf::Event& event ) override;
	void print( const std::string& message, sf::Color color = sf::Color::White );
	void printError( const ErrorLevel errorLevel, const std::string& description )
	{
		// Concatenate error level info with description
		std::string concatenated;
		switch ( errorLevel )
		{
			case ErrorLevel::WARNING:
				concatenated.assign( "WARNING: " + description );
				break;
			case ErrorLevel::CRITICAL:
				concatenated.assign( "CRITICAL: " + description );
				break;
			default:
#ifdef _DEBUG
				__debugbreak( );
#elif
				__assume( 0 );
				break;
#endif
		}
		print( concatenated, sf::Color::Red );
	}
	void printScriptError( const ErrorLevel errorLevel, const std::string& variableName, const std::string& scriptName )
	{
		printError( errorLevel, "Variable '" + variableName + "' in " + scriptName + " can't be found or has an odd value." );
	}
	/*void addCommand( const std::string& command,
					 std::function< void( void ) > function,
					 CommandType type,
					 const std::string& description )
	{
		mDictionary.emplace( command, function );
	};*/
private:
	void refresh( )
	{
		//TODO
	}

	bool mInitialized;
	bool mVisible;
	std::string mCurrentInput;
	///std::string mCursorForeground;
	sf::Font mFont;
	sf::RectangleShape mConsoleWindow;
	sf::Text mCurrentInputTextField;
	sf::Text mCursorForegroundTextField;
	std::array< sf::Text, 9 > mHistoryTextFields;

	///std::unordered_map< std::string, std::function< void( void ) > > mDictionary;//TODO: ¶¼¾îµÎ±â
};