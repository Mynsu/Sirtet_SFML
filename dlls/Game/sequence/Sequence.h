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

		// Pseudo-unnamed function
		void _2436549370( const std::string_view& args ); //궁금: 인라인함수는 안 되려나?

		void update( );
		void draw( );
	private:
		void setSequence( const ::sequence::Seq nextSequence );
		// A single instance can live at a time, two or more can't.
		// NOTE: Global access isn't necessary here.
		static bool IsInstanciated;
		// NOTE: On spatial locality members win, statics lose; locality leads to better performance.
		sf::RenderWindow* mWindow;
		std::unique_ptr< ::sequence::ISequence > mCurrentSequence;
		const std::function< void( const ::sequence::Seq ) > mSetter;
	};
}