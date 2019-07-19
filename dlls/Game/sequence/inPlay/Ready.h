#pragma once
#pragma hdrstop
#include "../../Common.h"
#include "ISequence.h"

namespace sequence::inPlay
{
	class GAME_API Ready final : public ::sequence::inPlay::ISequence
	{
	public:
		Ready( ) = delete;
		Ready( sf::RenderWindow& window, ::sequence::inPlay::Seq* const nextInPlaySequence, sf::Drawable& shapeOrSprite );
		~Ready( ) = default;

		void update( );
		void draw( );
	private:
		uint16_t mFrameRate;
		uint16_t mCountDown;
		sf::RenderWindow& mWindow;
		::sequence::inPlay::Seq* const mNextInPlaySequence;
		sf::RectangleShape& mBackgroundRect;
		sf::Vector2u mSpriteClipSize;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}