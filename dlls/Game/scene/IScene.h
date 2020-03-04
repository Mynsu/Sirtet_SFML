////
// 가장 바깥 씬들(인트로, 메인메뉴, 싱글 플레이 시퀀스, 온라인 배틀 시퀀스)의 인터페이스입니다.
// ::scene::inPlay::IScene.h, ::scene::online::IScene.h와 다릅니다.
////

#pragma once
#include <list>
#include <functional>
#include <SFML/Window.hpp>

namespace scene
{
	enum class ID
	{
		AS_IS = -1,
		INTRO = 0,
		MAIN_MENU = 1,
		SINGLE_PLAY = 2,
		ONLINE_BATTLE = 3,
	};

	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;

		virtual void loadResources( const sf::RenderWindow& window ) = 0;
		virtual ::scene::ID update( std::vector<sf::Event>& eventQueue ) = 0;
		virtual void draw( sf::RenderWindow& window ) = 0;
		// NOTE: 비싼 RTTI를 쓰지 않았습니다.
		virtual ::scene::ID currentScene( ) const = 0;
		virtual void setScene( const uint8_t sceneID ) = 0;

		// NOTE: 가시성을 protected로 제한하여
		// 인터페이스의 인스턴스가 구체concrete 클래스에 암시적으로 만들어지는 것 말고
		// 다른 방법으로는 만들 수 없도록 했습니다.
	protected:
		IScene( ) = default;
	};
}
