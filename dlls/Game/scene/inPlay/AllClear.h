#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class AllClear final : public IScene
	{
	private:
		static bool IsInstantiated;
	public:
		AllClear( sf::RenderWindow& window );
		~AllClear( );

		void loadResources( sf::RenderWindow& window ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
	private:
		struct
		{
			uint8_t confettiSpriteClipRow, confettiSpriteClipRowMax_,
				confettiSpriteClipColumn, confettiSpriteClipColumnMax_;
			float confettiRelativePlaySpeed;
			sf::Vector2i confettiSpriteClipSize;
		} mDrawingInfo;
		Clock::time_point mTimePreviousClipDraws;
		std::string mBGMPath;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}