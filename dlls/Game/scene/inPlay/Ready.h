#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class Ready final : public ::scene::inPlay::IScene
	{
	public:
		Ready( sf::RenderWindow& window, sf::Drawable& shapeOrSprite );
		~Ready( ) = default;

		void loadResources( sf::RenderWindow& window ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue )override;
		void draw( sf::RenderWindow& window ) override;
	private:
		uint16_t mFPS_;
		int32_t mFrameCountToStart;
		sf::RectangleShape& mBackgroundRect_;
		sf::Vector2f mSpriteClipSize_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}