#pragma once
#include "../IScene.h"
#include "IScene.h"

namespace scene::inPlay
{
	class InPlay final : public ::scene::IScene
	{
	private:
		static bool IsInstantiated;
	public:
		InPlay( sf::RenderWindow& window,
			   const ::scene::inPlay::ID initScene = ::scene::inPlay::ID::READY );
		~InPlay( );

		void loadResources( const sf::RenderWindow& window ) override;
		::scene::ID update( std::vector<sf::Event>& eventQueue ) override;
		void draw( sf::RenderWindow& window ) override;
		::scene::ID currentScene( ) const override;
		void setScene( const uint8_t sceneID ) override
		{
			setScene( (::scene::inPlay::ID)sceneID );
		}
	private:
		void setScene( const ::scene::inPlay::ID nextInPlaySceneID );
		uint16_t mFPS_;
		// NOTE: 읽기 쉽게 멤버 함수 update(...)의 인자로 넘기려니
		// 오버라이드하고 있는 순수 가상 함수까지 바꿔야해서
		// 생성자로 레퍼런스를 받아 쥐고 있습니다.
		sf::RenderWindow& mWindow_;
		std::unique_ptr<::scene::inPlay::IScene> mCurrentScene;
		// 동시에 여러 씬을 띄워 봤습니다.
		std::unique_ptr<::scene::inPlay::IScene> mOverlappedScene;
		sf::RectangleShape mBackground;
	};
}