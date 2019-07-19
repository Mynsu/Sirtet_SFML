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
		// A single instance can live at a time, two or more can't.
		// NOTE: Singleton pattern and service locator pattern also give this,
		//		 but global access isn't necessary here.
		static bool IsInstanciated;
		::sequence::Seq mOnIndicator;
		::sequence::Seq* const mNextMainSequence;
		sf::RenderWindow& mWindow;
		sf::Vector2u mSpriteClipSize;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}