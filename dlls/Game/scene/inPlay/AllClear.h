#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class AllClear final : public IScene
	{
	public:
		AllClear( sf::RenderWindow& window );
		~AllClear( );

		void loadResources( ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		static bool IsInstantiated;
		struct
		{
			uint8_t confettiSpriteClipRow, confettiSpriteClipRowMax_,
				confettiSpriteClipColumn, confettiSpriteClipColumnMax_;
			float confettiRelativePlaySpeed;
			sf::Vector2i confettiSpriteClipSize;
		} mDrawingInfo;
		Clock::time_point mTimePreviousClipDraws;
		sf::RenderWindow& mWindow_;
		std::string mBGMPath;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}