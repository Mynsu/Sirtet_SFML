////
// Game API
////
// If you want to get the callback function to load in .exe,
// please look at this .h file, not the others.
////

#pragma once
#include "../exes/Engine/ServiceLocator.h"

struct EngineComponents;

class IGame
{
	friend void _2943305454( const EngineComponents );
public:
	virtual ~IGame( ) = default;

	virtual void update( std::list< sf::Event >& eventQueue ) = 0;
	virtual void draw( ) = 0;
protected:
	IGame( ) = default;
	IGame( const IGame& ) = delete;
	void operator=( const IGame& ) = delete;
private:
	// NOTE: Only is this called in the friendly(?) pseudo-unnamed function '_2943305454(...)',
	//		 which leads to safe usage.
	virtual void setWindow( sf::RenderWindow* const window ) = 0;
};

// Consists of pointers.
struct EngineComponents
{
	ServiceLocator* service;
	sf::RenderWindow* window;
};

// Just alias.
using Game = IGame;
// Consists of pointers.
struct GameComponents
{
	Game* game;
};

using GetGameAPI_t = const GameComponents (*)( const EngineComponents );
