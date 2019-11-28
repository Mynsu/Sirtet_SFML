#pragma once
#include <list>
#include <SFML/Window.hpp>

namespace scene::online
{
	enum class ID
	{
		MAIN_MENU = -2,
		AS_IS = -1,
		WAITING,
		IN_LOBBY,
		IN_ROOM_AS_HOST,
		IN_ROOM,
	};

	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;

		virtual void loadResources( ) = 0;
		// Returns 0 when doing nothing, -1 when coming back, 1 when going on.
		virtual ::scene::online::ID update( std::list< sf::Event >& eventQueue ) = 0;
		virtual void draw( ) = 0;
	protected:
		IScene( ) = default;
	};
}