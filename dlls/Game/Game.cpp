#include "pch.h"
#include "Game.h"
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
		: mWindow( nullptr )
	{
		IsInstantiated = true;
	}
	~GameLocal( )
	{
		mWindow = nullptr;
		IsInstantiated = false;
	};

	void update( std::list< sf::Event >& eventQueue )
	{
		mSceneManager.update( eventQueue );
	}
	void draw( )
	{
		mSceneManager.draw( );
	}
private:
	void setWindow( sf::RenderWindow* const window )
	{
		mWindow = window;
		mSceneManager.lastInit( window );
	}

	sf::RenderWindow* mWindow;
	::scene::SceneManager mSceneManager;
};

bool GameLocal::IsInstantiated = false;

// NOTE: In this way an instance will be created in compile-time,
//		 but often needs another initialization in runtime.
std::unique_ptr< IGame > _Game( std::make_unique< GameLocal >( ) );

// Pseudo-unnamed function
inline void _2943305454( const EngineComponents engine )
{
	glpService.mPtr = engine.service;
	// !IMPORTANT: Call this only after all the engine components has been linked, or assigned like above.
	_Game->setWindow( engine.window );
}

extern "C" const GameComponents GetGameAPI( const EngineComponents engine )
{
	_2943305454( engine );
	GameComponents ret;
	ret.game = _Game.get( );
	return ret;
}