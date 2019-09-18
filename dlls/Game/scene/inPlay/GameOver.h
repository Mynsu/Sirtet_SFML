#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "IScene.h"

namespace scene::inPlay
{
	class GameOver : public ::scene::inPlay::IScene
	{
	public:
		GameOver( ) = delete;
		GameOver( sf::RenderWindow& window, sf::Drawable& shapeOrSprite );
		~GameOver( ) = default;

		void loadResources( ) override;
		int8_t update( ::scene::inPlay::IScene** const ignored, std::list< sf::Event >& ignored_eventQueue ) override;
		void draw( ) override;
	private:
		const uint8_t TARGET_ALPHA;
		uint32_t mFade;
		uint32_t mFPS;
		uint32_t mFrameCount;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}