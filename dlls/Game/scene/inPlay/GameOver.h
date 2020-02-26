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
		// NOTE: �б� ���� ��� �Լ� draw(...)�� ���ڷ� �ѱ����
		// �������̵��ϰ� �ִ� ���� ���� �Լ����� �ٲ���ؼ�
		// �����ڷ� ���۷����� �޾� ��� �ֽ��ϴ�.
		sf::RectangleShape& mBackgroundRect_;
		sf::Texture mTexture;
		std::string mSoundPaths[(int)SoundIndex::NULL_MAX];
		sf::Sprite mSprite;
	};
}