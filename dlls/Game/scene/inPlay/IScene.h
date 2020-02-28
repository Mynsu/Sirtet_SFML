////
// 인플레이 씬들(도입부 카운트다운, 싱글 플레이, 게임 오버, 올 클리어, 나가려 할 때 다시 묻기 등)의 인터페이스입니다.
// 이 씬들은 싱글 플레이 전용이 아니고 실제로도 온라인 배틀 시퀀스에서 쓰이고 있다는 것에 유의해야 합니다.
// ::scene::IScene.h, ::scene::online::IScene.h와 다릅니다.
////
#pragma once
#include <list>
#include <SFML/Window.hpp>

namespace scene::inPlay
{
	enum class ID
	{
		EXIT = -3,
		UNDO = -2,
		AS_IS = -1,
		READY,
		PLAYING,
		GAME_OVER,
		ALL_CLEAR, // All-levels-clear scene.
		ASSERTION,
	};

	class IScene
	{
	public:
		IScene( const IScene& ) = delete;
		void operator=( const IScene& ) = delete;
		virtual ~IScene( ) = default;
		
		virtual void loadResources( sf::RenderWindow& window ) = 0;
		virtual ::scene::inPlay::ID update( std::vector<sf::Event>& eventQueue ) = 0;
		virtual void draw( sf::RenderWindow& window ) = 0;

		// NOTE: 가시성을 protected로 제한하여
		// 인터페이스의 인스턴스가 구체concrete 클래스에 암시적으로 만들어지는 것 말고
		// 다른 방법으로는 만들 수 없도록 했습니다.
	protected:
		IScene( ) = default;
	};
}