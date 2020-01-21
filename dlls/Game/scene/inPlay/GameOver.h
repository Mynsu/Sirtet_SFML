#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class GameOver final : public ::scene::inPlay::IScene
	{
	public:
		GameOver( ) = delete;
		GameOver( sf::RenderWindow& window, sf::Drawable& shapeOrSprite, std::unique_ptr<::scene::inPlay::IScene>& overlappedScene );
		~GameOver( ) = default;

		void loadResources( ) override;
		::scene::inPlay::ID update( std::list< sf::Event >& ignored_eventQueue ) override;
		void draw( ) override;
	private:
		const uint8_t TARGET_ALPHA;
		uint32_t mFade;
		uint32_t mFPS_, mFrameCount;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}