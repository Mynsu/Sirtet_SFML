#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>

namespace scene::inPlay
{
	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;
		
		virtual void loadResources( ) = 0;
		virtual void update( ::scene::inPlay::IScene** const, std::queue< sf::Event >& eventQueue ) = 0;
		virtual void draw( ) = 0;

		// NOTE: Protected constructor prevents users from instantiating the abstract class.
	protected:
		IScene( ) = default;
	};
}