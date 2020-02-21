#include "pch.h"
#include "ServiceLocatorMirror.h"
#include "scene/SceneManager.h"

class GameLocal final : public IGame
{
private:
	// Only a single instance for a type can live at a time, but shouldn't be accessible globally.
	// That's the difference from the class filled with static functions, or on singleton pattern and the like.
	//
	// NOTE: 'private static' is better than one in an unnamed namespace
	// because in this .cpp file there are much more than this class.
	static bool IsInstantiated;
public:
	// NOTE: Called in compile-time, thus another initialization should be done in runtime.
	GameLocal( )
	{
		IsInstantiated = true;
	}
	~GameLocal( )
	{
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

// NOTE: In this way an instance will be created in compile-time,
//		 but often needs another initialization in runtime.
std::unique_ptr<IGame> _Game( std::make_unique<GameLocal>() );

// Pseudo-unnamed function
inline void _2943305454( const EngineComponents engine )
{
	// Prerequisite for _Game->init( ).
	gService.mPtr = engine.service;
	_Game->init( *engine.window );
}

extern "C" const GameComponents GetGameAPI( const EngineComponents engine )
{
	_2943305454( engine );
	GameComponents ret;
	ret.game = _Game.get( );
	return ret;
}