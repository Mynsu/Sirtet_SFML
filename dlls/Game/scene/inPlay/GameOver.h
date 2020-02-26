#pragma once
#include "IScene.h"

namespace scene::inPlay
{
	class GameOver final : public ::scene::inPlay::IScene
	{
	private:
		static bool IsInstantiated;
	public:
		GameOver( sf::RenderWindow& window, sf::Drawable& shapeOrSprite,
				 std::unique_ptr<::scene::inPlay::IScene>& overlappedScene );
		~GameOver( );

		void loadResources( sf::RenderWindow& window ) override;
		::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
	private:
		enum class SoundIndex
		{
			BGM,
			NULL_MAX,
		};
		uint16_t mFPS_, mFrameCountToMainMenu;
		uint32_t mFade;
		// NOTE: 읽기 쉽게 멤버 함수 draw(...)의 인자로 넘기려니
		// 오버라이드하고 있는 순수 가상 함수까지 바꿔야해서
		// 생성자로 레퍼런스를 받아 쥐고 있습니다.
		sf::RectangleShape& mBackgroundRect_;
		sf::Texture mTexture;
		std::string mSoundPaths[(int)SoundIndex::NULL_MAX];
		sf::Sprite mSprite;
	};
}