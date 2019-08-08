#pragma once
#pragma hdrstop
#include "ISequence.h"
#include <functional>

namespace sequence
{
	class Sequence
	{
	public:
		// NOTE: Called in compile-time, thus requires 'lastInit(...)',
		//		 which will be called in 'GetGameAPI(...)' in 'Game.cpp.'
		Sequence( );
		~Sequence( )
		{
			IsInstanciated = false;
			mWindow = nullptr;
		};
		Sequence( const Sequence& ) = delete;
		void operator=( const Sequence& ) = delete;
		void lastInit( sf::RenderWindow* const window );


		void update( );
		void draw( );
	private:
		// Pseudo-unnamed function
		// NOTE: A few complier may confuse this with an old inline function(just copied)
		//		 and cause an failure in 'addCommand(...).'
		void _2436549370( const std::string_view& args )
		{
			setSequence( static_cast< ::sequence::Seq >( std::atoi( args.data( ) ) ) );
		}
		void setSequence( const ::sequence::Seq nextSequence );
		// A single instance can live at a time, two or more can't.
		// NOTE: Global access isn't necessary here.
		static bool IsInstanciated;
		// NOTE: On spatial locality members win, statics lose; locality leads to better performance.
		sf::RenderWindow* mWindow;
		std::unique_ptr< ::sequence::ISequence > mCurrentSequence;
		SetSequence_t mSetter;
	};
}