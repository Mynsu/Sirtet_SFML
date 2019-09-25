#pragma once
#include "Game.h"
#include "../exes/Engine/Console.h"

class ServiceLocatorMirror
{
	friend void _2943305454( const EngineComponents );
public:
	inline ServiceLocatorMirror( )
		: mPtr( nullptr )
	{
		ASSERT_FALSE( IsInstantiated );
		IsInstantiated = true;
	}
	inline ~ServiceLocatorMirror( )
	{
		mPtr = nullptr;
		IsInstantiated = false;
	}

	::ServiceLocator& operator*( )
	{
		return *mPtr;
	}
private:
	static bool IsInstantiated;
	::ServiceLocator* mPtr;
};

extern ::ServiceLocatorMirror glpService;