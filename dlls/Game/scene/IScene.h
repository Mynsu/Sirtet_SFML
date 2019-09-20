#pragma once
#include <list>
#include <functional>
#include <SFML/Window.hpp>

namespace scene
{
	enum class ID
	{
		INTRO = 0,
		MAIN_MENU,
		SINGLE_PLAY,
		ONLINE_BATTLE,
		MAX_NONE,
	};

	using SetScene_t = std::function< void( const ::scene::ID ) >;

	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;

		virtual void loadResources( ) = 0;
		virtual void update( std::list< sf::Event >& eventQueue ) = 0;
		virtual void draw( ) = 0;
		virtual ::scene::ID currentScene( ) const = 0;

		// NOTE: Protected constructor prevents users from instantiating the abstract class.
	protected:
		IScene( ) = default;
	};
}
