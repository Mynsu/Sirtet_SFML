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
		void update( ::scene::inPlay::IScene** const nextScene, std::queue< sf::Event >& eventQueue ) override;
		void draw( ) override;
	private:
		uint32_t mFade;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}