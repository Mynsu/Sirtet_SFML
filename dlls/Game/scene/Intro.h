#pragma once
#include "IScene.h"

namespace scene
{
	class Intro final : public ::scene::IScene
	{
	private:
		// �� �̻��� �ν��Ͻ��� ���� �� �����ϴ�.
		static bool IsInstantiated;
	public:
		Intro( sf::RenderWindow& window );
		~Intro( );

		void loadResources( sf::RenderWindow& window ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
		::scene::ID currentScene( ) const override;
		void setScene( const uint8_t sceneID ) override
		{}
	private:
		// Time Unit: Seconds.
		const uint8_t mDuration;
		uint8_t mAlpha;
		uint16_t mFPS_, mFrameCountToStart;
		::scene::ID mNextScene;
		sf::Texture mTexture;
		sf::Sprite mSprite;
	};
}
