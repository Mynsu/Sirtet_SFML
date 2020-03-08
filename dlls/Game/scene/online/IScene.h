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
	protected:
		IScene( ) = default;
	};
}