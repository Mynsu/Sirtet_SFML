#pragma once
#include <list>
#include <functional>
#include <SFML/Window.hpp>

namespace scene
{
	enum class ID
	{
		AS_IS = -1,
		INTRO = 0,
		MAIN_MENU = 1,
		SINGLE_PLAY = 2,
			ALL_LEVELS_CLEARED = 21,
		ONLINE_BATTLE = 3,
	};

	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;

		virtual void loadResources( ) = 0;
		virtual ::scene::ID update( std::vector<sf::Event>& eventQueue ) = 0;
		virtual void draw( ) = 0;
		virtual ::scene::ID currentScene( ) const = 0;
		virtual void setScene( const uint8_t sceneID ) = 0;

		// NOTE: Protected constructor prevents users from instantiating the abstract class.
	protected:
		IScene( ) = default;
	};
}
