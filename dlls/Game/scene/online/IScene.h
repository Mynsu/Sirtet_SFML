////
// 온라인 플레이 씬들(대기열, 로비, 방)의 인터페이스입니다.
// ::scene::IScene.h, ::scene::inPlay::IScene.h와 다릅니다.
////

#pragma once
#include <list>
#include <SFML/Window.hpp>

namespace scene::online
{
	class Online;

	enum class ID
	{
		MAIN_MENU = -2,
		AS_IS = -1,
		WAITING,
		IN_LOBBY,
		IN_ROOM_AS_HOST,
		IN_ROOM,
	};

	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;

		virtual void loadResources( const sf::RenderWindow& window ) = 0;
		virtual ::scene::online::ID update( std::vector<sf::Event>& eventQueue,
										   ::scene::online::Online& net,
										   const sf::RenderWindow& window ) = 0;
		virtual void draw( sf::RenderWindow& window ) = 0;

		// NOTE: 가시성을 protected로 제한하여
		// 인터페이스의 인스턴스가 구체concrete 클래스에 암시적으로 만들어지는 것 말고
		// 다른 방법으로는 만들 수 없도록 했습니다.
	protected:
		IScene( ) = default;
	};
}