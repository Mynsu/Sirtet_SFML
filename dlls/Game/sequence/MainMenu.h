#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class GAME_API MainMenu final : public ::sequence::ISequence
	{
	public:
		MainMenu( ) = delete;
		MainMenu( sf::RenderWindow& window,
				  ::sequence::Seq* const nextMainSequence );
		~MainMenu( )
		{
			IsInstanciated = false;
		}

		void update( );
		void draw( );
	private:
		// Only single instance can live, two or more can't.
		static bool IsInstanciated;
		::sequence::Seq mOnIndicator;
	};
}