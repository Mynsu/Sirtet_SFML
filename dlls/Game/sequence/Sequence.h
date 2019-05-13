#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class GAME_API Sequence
	{
	public:
		Sequence( sf::RenderWindow& window );
		~Sequence( ) = default;

		void update( );
		void draw( );
	private:
		sf::RenderWindow& const mWindow;
		::sequence::Seq* const mNextSequence;
		std::unique_ptr< ::sequence::ISequence > mCurrentSequence;
	};
}