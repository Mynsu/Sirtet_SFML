#pragma once
#include <list>
#include <SFML/Window.hpp>

namespace scene::online
{
	class Online;

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

		virtual void loadResources( sf::RenderWindow& window ) = 0;
		// Returns 0 when doing nothing, -1 when coming back, 1 when going on.
		virtual ::scene::online::ID update( std::vector<sf::Event>& eventQueue,
										   ::scene::online::Online& net,
										   sf::RenderWindow& window ) = 0;
		virtual void draw( sf::RenderWindow& window ) = 0;
	protected:
		IScene( ) = default;
	};
}