#pragma once
#pragma hdrstop
#include <Lib/precompiled.h>
#include "Game.h"
#include "../exes/Engine/Console.h"

class ServiceLocatorMirror
{
	friend void _2943305454( const EngineComponents );
public:
	ServiceLocatorMirror( ) = delete;
	~ServiceLocatorMirror( ) = delete;

	static auto Console( ) -> const std::unique_ptr< IConsole >&
	{
		return _Console( );
	}
	static auto Vault( ) -> std::unordered_map< HashedKey, Dword >&
	{
		return _Vault( );
	}
	static void Release( )
	{
		_Console = nullptr;
		_Vault = nullptr;
	}
private:
	static GetConsole_t _Console;
	static GetVault_t _Vault;
};