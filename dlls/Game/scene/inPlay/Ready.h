#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "IScene.h"

namespace scene::inPlay
{
	class Ready final : public ::scene::inPlay::IScene
	{
	public:
		Ready( ) = delete;
		Ready( sf::RenderWindow& window, sf::Drawable& shapeOrSprite );
		~Ready( ) = default;

		void loadResources( ) override;
		void update( ::scene::inPlay::IScene** const nextScene, std::queue< sf::Event >& eventQueue ) override;
		void draw( ) override;
	private:
		uint16_t mFPS_;
		uint16_t mFrameCount;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		sf::Vector2u mSpriteClipSize_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}