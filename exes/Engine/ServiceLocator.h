#pragma once
#pragma hdrstop
#include "Lib/precompiled.h"
#include "Console.h"

class ServiceLocator
{
public:
	ServiceLocator( ) = delete;
	~ServiceLocator( ) = delete;

	// Access console.
	static auto Console( ) -> const std::unique_ptr< IConsole >&
	{
		return _Console;
	}
	// Access the variable table.
	static auto Vault( ) -> std::unordered_map< HashedKey, Dword >&
	{
		return _Vault;
	}
private:
	static std::unique_ptr< IConsole > _Console;//TODO: 콘솔을 개발용으로만 둘까, 콘솔에 유저 권한을 둘까?
	static std::unordered_map< HashedKey, Dword > _Vault;
};	