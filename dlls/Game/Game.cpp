#include "pch.h"
#include <GameLib/IGame.h>
#include "ServiceLocatorMirror.h"
#include "scene/SceneManager.h"

class GameLocal final : public IGame
{
private:
	// �� �̻��� �ν��Ͻ��� ���� �� �����ϴ�.
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

// Ŭ���� ServiceLocatorMirror�� �߻� Ŭ���� IGame�� friend �Լ��Դϴ�.
// ���� ������� mPtr, ����Լ� init(...)�� ���߰� �� �Լ������� �����մϴ�.
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