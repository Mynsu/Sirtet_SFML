#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class Ready final : public ::scene::inPlay::IScene
	{
	private:
		static bool IsInstantiated;
	public:
		Ready( const sf::RenderWindow& window, sf::Drawable& shapeOrSprite );
		~Ready( );

		void loadResources( const sf::RenderWindow& window ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
	private:
		uint16_t mFPS_;
		int32_t mFrameCountToStart;
		sf::RectangleShape& mBackgroundRect_;
		sf::Vector2f mSpriteClipSize;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}