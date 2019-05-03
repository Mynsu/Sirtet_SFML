#pragma once
///#pragma hdrstop
#include <array>
#include <string_view>
#include <functional>
#include <string>
#include <SFML/Graphics.hpp>

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
	virtual void print( const std::string& message ) = 0;
	virtual void printError( const std::string& errorMessage ) = 0;
	virtual void addCommand( std::string_view command,
							 std::function< void( void ) > function,
							 CommandType type,
							 std::string_view description ) = 0;//미완
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
	void print( const std::string& message );
	void printError( const std::string& errorMessage );
	//궁금: 왜 virtual일까?
	void addCommand( std::string_view command,
					 std::function< void( void ) > function,
					 CommandType type,
					 std::string_view description ) override;//
private:
	bool mVisible;
	bool mInitialized;
	std::string mCurrentInput;
	std::string mCursorForeground; // 28
	sf::Font mFont; // 76
	sf::RectangleShape mConsoleWindow; // 292
	sf::Text mCurrentInputTextField; // 312
	sf::Text mCursorForegroundTextField;
	std::array< sf::Text, 9 > mHistoryTextFields;
};