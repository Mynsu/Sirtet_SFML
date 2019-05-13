#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class GAME_API MainMenu final : public ISequence
	{
	public:
		MainMenu( ) = delete;
		MainMenu( sf::RenderWindow& window,
				  ::sequence::Seq* const nextSequence );
		~MainMenu( )
		{
			IsInstanciated = false;
		}

		void update( );
		void draw( );
	private:
		// Only single instance can live, two or more can't.
		static bool IsInstanciated;
		::sequence::Seq* const mNextSequence;
		sf::RenderWindow& const mWindow;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}