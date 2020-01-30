#pragma once
#include <list>
#include <functional>
#include <SFML/Window.hpp>

namespace scene
{
	enum class ID
	{
		AS_IS = -1,
		INTRO,
		MAIN_MENU,
		SINGLE_PLAY,
		ONLINE_BATTLE,
		MAX,
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
#ifdef _DEV
		virtual ::scene::ID currentScene( ) const = 0;
#endif

		// NOTE: Protected constructor prevents users from instantiating the abstract class.
	protected:
		IScene( ) = default;
	};
}
