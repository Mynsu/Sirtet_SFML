#pragma once
#include <list>
#include <stdint.h>
#include <SFML/Window.hpp>

namespace scene::inPlay
{
	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;
		
		virtual void loadResources( ) = 0;
		// Returns 0 when doing nothing, -1 when coming back, 1 when going on.
		virtual int8_t update( ::scene::inPlay::IScene** const, std::list< sf::Event >& eventQueue ) = 0;
		virtual void draw( ) = 0;

		// NOTE: Protected constructor prevents users from instantiating the abstract class.
	protected:
		IScene( ) = default;
	};
}