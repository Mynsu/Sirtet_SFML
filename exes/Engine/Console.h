#pragma once
//TODO:
//#include <memory>
#include <string_view>
#include <functional>
#include <SFML\Graphics.hpp>

//TODO: 인터페이스를 다른 헤더 파일에 놓을까?
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

	virtual void addCommand( std::string_view command,
							 std::function< void( void ) > function,
							 CommandType type,
							 std::string_view description ) = 0;
	virtual void draw( sf::RenderTarget& target, sf::RenderStates states ) const = 0;
};

class ConsoleLocal final : public IConsole
{
public:
	ConsoleLocal( );
	~ConsoleLocal( ) = default;

	//궁금: 왜 virtual일까?
	void addCommand( std::string_view command,
					 std::function< void( void ) > function,
					 CommandType type,
					 std::string_view description ) override;

	void draw( sf::RenderTarget& target, sf::RenderStates states ) const;
};

//TODO:
//extern std::unique_ptr< IConsole > Console;