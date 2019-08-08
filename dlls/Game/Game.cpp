#include "Game.h"
#include "Lib/precompiled.h"
#include "sequence/Sequence.h"

class GameLocal final : public IGame
{
public:
	// NOTE: Called in compile-time.
	GameLocal( )
		: mWindow( nullptr )
	{
		IsInstanciated = true;
	}
	~GameLocal( )
	{
		mWindow = nullptr;
		ServiceLocatorMirror::Release( );
	};

	void update( )
	{
		mSequence.update( );
	}
	void draw( )
	{
		mSequence.draw( );
	}
private:
	void setWindow( sf::RenderWindow* const window )
	{
		mWindow = window;
		mSequence.lastInit( window );
	}

	//궁금: 베이스에다 놓을까?
	// A single instance can live at a time, two or more can't.
	// NOTE: Global access isn't necessary here.
	static bool IsInstanciated;
	// NOTE: On spatial locality members win, statics lose; locality leads to better performance.
	sf::RenderWindow* mWindow;
	::sequence::Sequence mSequence;
};

bool GameLocal::IsInstanciated = false;

// NOTE: In this way an instance will be created in compile-time,
//		 but can require another initialization, which should inevitably be in runtime.
std::unique_ptr< IGame > _Game( std::make_unique< GameLocal >( ) );

// Pseudo-unnamed function
inline void _2943305454( const EngineComponents engine )
{
	// !IMPORTANT: Order
	ServiceLocatorMirror::_Console = engine.console;
	ServiceLocatorMirror::_Vault = engine.vault;
	_Game->setWindow( engine.window );
}

extern "C" const GameComponents GetGameAPI( const EngineComponents engine )
{
	_2943305454( engine );
	GameComponents ret;
	ret.game = _Game.get( );
	return ret;
}