#pragma once
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
		int8_t update( ::scene::inPlay::IScene** const nextScene, std::list< sf::Event >& ignored_eventQueue ) override;
		void draw( ) override;
	private:
		uint32_t mFPS_;
		int32_t mFrameCount;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		sf::Vector2f mSpriteClipSize_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}