#include "pch.h"
#include <GameLib/IGame.h>
#include "ServiceLocatorMirror.h"
#include "scene/SceneManager.h"

class GameLocal final : public IGame
{
private:
	// 둘 이상의 인스턴스를 만들 수 없습니다.
	static bool IsInstantiated;
public:
	// Needs init(...) to be called.
	GameLocal( )
	{
		IsInstantiated = true;

		WSAData w;
		WSAStartup( MAKEWORD(2, 2), std::addressof(w) );
	}
	~GameLocal( )
	{
		WSACleanup();

		IsInstantiated = false;
	};

	void update( std::vector<sf::Event>& eventQueue )
	{
		mSceneManager.update( eventQueue );
	}
	void draw( )
	{
		mSceneManager.draw( );
	}
private:
	void init( sf::RenderWindow& window )
	{
		mSceneManager.init( window );
	}

	::scene::SceneManager mSceneManager;
};

bool GameLocal::IsInstantiated = false;

std::unique_ptr<IGame> _Game( std::make_unique<GameLocal>() );

// 클래스 ServiceLocatorMirror와 추상 클래스 IGame의 friend 함수입니다.
// 각각 멤버변수 mPtr, 멤버함수 init(...)을 감추고 이 함수에서만 접근합니다.
// Pseudo-unnamed function
inline void _2943305454( const EngineComponents engine )
{
	// Prerequisite for _Game->init( ).
	gService.mPtr = engine.service;
	ASSERT_NOT_NULL( _Game );
	_Game->init( *engine.window );
}

extern "C" const GameComponents GetGameAPI( const EngineComponents engine )
{
	_2943305454( engine );
	GameComponents ret;
	ret.game = _Game.get( );
	return ret;
}