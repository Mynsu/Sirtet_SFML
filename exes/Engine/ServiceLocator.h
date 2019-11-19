#pragma once
#include "pch.h" // Commonly included headers, not pch.
#include "Console.h"

class ServiceLocator : public IServiceLocator
{
public:
	inline ServiceLocator( )
	{
		ASSERT_FALSE( IsInstantiated );

		WSAData w;
		WSAStartup( MAKEWORD(2, 2), &w );

		IsInstantiated = true;
	}
	ServiceLocator( const ServiceLocator& ) = delete;
	void operator=( const ServiceLocator& ) = delete;
	inline ~ServiceLocator( )
	{
		WSACleanup();

		IsInstantiated = false;
	}

	// Access the console.  For API.
	IConsole* console( ) override
	{
		return &mConsole;
	}
	// Access the console.  For internal use.
	inline Console& _console( )
	{
		return mConsole;
	}
	// Access global variables.
	auto vault( ) -> std::unordered_map< HashedKey, Dword >&
	{
		return mVault;
	}
	void release( )
	{
		// !IMPORTANT: MUST NOT get rid of this line.
		mConsole.release( );
	}
private:
	static bool IsInstantiated;
//TODO: 콘솔을 개발용으로만 둘까, 콘솔에 유저 권한을 둘까?
	Console mConsole;
	std::unordered_map< HashedKey, Dword > mVault;
};

extern ::ServiceLocator gService;