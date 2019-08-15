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
		virtual void update( std::unique_ptr< ::scene::inPlay::IScene >* const ) = 0;
		virtual void draw( ) = 0;

		// NOTE: Protected constructor prevents users from instantiating the abstract class.
	protected:
		IScene( ) = default;
	};
}