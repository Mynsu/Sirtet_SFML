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
		inline Sequence( )
			: mWindow( nullptr ), mSetter( std::bind( &::sequence::Sequence::setSequence, this, std::placeholders::_1 ) )
		{
			ASSERT_FALSE( IsInstanciated );
			IsInstanciated = true;
		}
		inline ~Sequence( )
		{
			IsInstanciated = false;
			mWindow = nullptr;
		}
		Sequence( const Sequence& ) = delete;
		void operator=( const Sequence& ) = delete;
		void lastInit( sf::RenderWindow* const window );

		inline void update( )
		{
			mCurrentSequence->update( );
		}
		inline void draw( )
		{
			mCurrentSequence->draw( );
		}
	private:
		void setSequence( const ::sequence::Seq nextSequence );
		// Pseudo-unnamed function
		void _2436549370( const std::string_view& args );
		// Pseudo-unnamed function
		void _495146883( const std::string_view& );
		// NOTE: On spatial locality members win, statics lose; locality leads to better performance.
		sf::RenderWindow* mWindow;
		SetSequence_t mSetter;
		std::unique_ptr< ::sequence::ISequence > mCurrentSequence;
	};
}