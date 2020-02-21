////
//  Commonly used in exes/Engine, dlls/Game.
////

#pragma once
#include "IServiceLocator.h"

// Consists of pointers.
struct EngineComponents
{
	IServiceLocator* service;
	sf::RenderWindow* window;
};

class IGame
{
	friend void _2943305454( const EngineComponents );
public:
	virtual ~IGame( ) = default;

	virtual void update( std::vector<sf::Event>& eventQueue ) = 0;
	virtual void draw( ) = 0;
protected:
	IGame( ) = default;
	IGame( const IGame& ) = delete;
	void operator=( const IGame& ) = delete;
private:
	// NOTE: Only is this called in the friendly(?) pseudo-unnamed function '_2943305454(...)',
	//		 which leads to safe usage.
	virtual void init( sf::RenderWindow& window ) = 0;
};

// Just alias.
using Game = IGame;
// Consists of pointers.
struct GameComponents
{
	Game* game;
};

using GetGameAPI_t = const GameComponents( *)( const EngineComponents );