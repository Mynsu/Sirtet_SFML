#pragma once
#include "Game.h"
#include "../exes/Engine/Console.h"

class ServiceLocatorMirror
{
	friend void _2943305454( const EngineComponents );
public:
	inline ServiceLocatorMirror( )
		: mPtr( nullptr )
	{ }
	inline ~ServiceLocatorMirror( )
	{
		mPtr = nullptr;
	}

	::ServiceLocator& operator*( )
	{
		return *mPtr;
	}
private:
	::ServiceLocator* mPtr;
};

extern ::ServiceLocatorMirror glpService;