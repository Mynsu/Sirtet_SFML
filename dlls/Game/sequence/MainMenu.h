#pragma once
#pragma hdrstop
#include "../Common.h"
#include "ISequence.h"

namespace sequence
{
	class MainMenu final : public ::sequence::ISequence
	{
	public:
		MainMenu( ) = delete;
		MainMenu( sf::RenderWindow& window );
		~MainMenu( )
		{
			IsInstanciated = false;
		}

		void update( );
		void draw( );
	private:
		// A single instance can live at a time, two or more can't.
		// NOTE: Global access isn't necessary here.
		static bool IsInstanciated;
		::sequence::Seq mOnIndicator;
		sf::RenderWindow& mWindow;
		sf::Vector2u mSpriteClipSize;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}