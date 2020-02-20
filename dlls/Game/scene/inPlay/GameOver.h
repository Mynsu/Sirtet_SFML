#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class GameOver final : public ::scene::inPlay::IScene
	{
	public:
		GameOver( sf::RenderWindow& window, sf::Drawable& shapeOrSprite, std::unique_ptr<::scene::inPlay::IScene>& overlappedScene );
		~GameOver( ) = default;

		void loadResources( ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( ) override;
	private:
		enum class SoundIndex
		{
			BGM,
			NULL_MAX,
		};
		const uint8_t TARGET_ALPHA;
		uint16_t mFPS_, mFrameCountToMainMenu;
		uint32_t mFade;
		sf::RenderWindow& mWindow_;
		sf::RectangleShape& mBackgroundRect_;
		sf::Texture mTexture;
		std::string mSoundPaths[(int)SoundIndex::NULL_MAX];
		sf::Sprite mSprite;
	};
}