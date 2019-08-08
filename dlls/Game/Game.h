#pragma once
#pragma hdrstop
#include "Common.h"

struct EngineComponents;

class IGame
{
	friend void _2943305454( const EngineComponents );
public:
	virtual ~IGame( ) = default;

	virtual void update( ) = 0;
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
	GetConsole_t console;
	GetVault_t vault;
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
