#pragma once
#include "IScene.h"

namespace scene
{
	class Intro final : public ::scene::IScene
	{
	private:
		// 둘 이상의 인스턴스를 만들 수 없습니다.
		static bool IsInstantiated;
	public:
		Intro( const sf::RenderWindow& window );
		~Intro( );

		void loadResources( const sf::RenderWindow& window ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue,
						   const sf::RenderWindow& window ) override;
		void draw( sf::RenderWindow& window ) override;
		::scene::ID currentScene( ) const override;
		void setScene( const uint8_t sceneID,
					  const sf::RenderWindow& ) override
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
