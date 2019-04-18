#pragma hdrstop
#pragma once
#include <string_view>
#include <functional>
#include <string>
#include <SFML\Graphics.hpp>

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

	virtual void draw( sf::RenderTarget& target, sf::RenderStates states ) const = 0;
	virtual void handleEvent( const sf::Event& event ) = 0;
	///virtual void update( ) = 0;
	virtual bool isVisible( ) const = 0;
	virtual void toggleVisible( ) = 0;
	virtual void addCommand( std::string_view command,
							 std::function< void( void ) > function,
							 CommandType type,
							 std::string_view description ) = 0;//
};

class ConsoleLocal final : public IConsole
{
public:
	ConsoleLocal( );
	~ConsoleLocal( ) = default;

	void draw( sf::RenderTarget& target, sf::RenderStates states ) const;
	void handleEvent( const sf::Event& event );
	///void update( );
	bool isVisible( ) const;
	void toggleVisible( );
	//±√±›: ø÷ virtual¿œ±Ó?
	void addCommand( std::string_view command,
					 std::function< void( void ) > function,
					 CommandType type,
					 std::string_view description ) override;//
private:
	bool mVisible;
	size_t mCursorPosition;//x,y
	sf::Font mFont;
	std::string mCurrentInput;
	std::string mCursorForeground;
	sf::Text mCurrentInputTextField;
	sf::Text mCursorForegroundTextField;
};