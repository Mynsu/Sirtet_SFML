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
		// NOTE: It looks odd at first glance
		//		 that a concept 'Ready' has an attribute 'mFPS.'
		//		 However, performance is prior to that.
		uint16_t mFPS;
		uint16_t mFrameCount;
		sf::RenderWindow& mWindow;
		sf::RectangleShape& mBackgroundRect;
		sf::Vector2u mSpriteClipSize;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}