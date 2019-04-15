#pragma once
//TODO:
//#include <memory>
#include <string_view>
#include <functional>
#include <SFML\Graphics.hpp>

//TODO: �������̽��� �ٸ� ��� ���Ͽ� ������?
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

	//�ñ�: �� virtual�ϱ�?
	void addCommand( std::string_view command,
					 std::function< void( void ) > function,
					 CommandType type,
					 std::string_view description ) override;

	void draw( sf::RenderTarget& target, sf::RenderStates states ) const;
};

//TODO:
//extern std::unique_ptr< IConsole > Console;