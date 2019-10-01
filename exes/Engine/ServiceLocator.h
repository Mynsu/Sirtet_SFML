#pragma once
#include "pch.h" // Commonly included headers, not pch.
#include "Console.h"

class ServiceLocator
{
public:
	inline ServiceLocator( )
		: mConsole( std::make_unique<ConsoleLocal>() )
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
		///mSocket.close();
		WSACleanup();

		IsInstantiated = false;
	}

	// Access console.
	auto console( ) -> const std::unique_ptr< IConsole >&
	{
		return mConsole;
	}
	// Access the variable table.
	auto vault( ) -> std::unordered_map< HashedKey, Dword >&
	{
		return mVault;
	}
	///auto socket( ) -> std::unique_ptr< Socket >&
	///{
	///	return mSocket;
	///}
	void release( )
	{
		// !IMPORTANT: MUST NOT get rid of this line.
		mConsole.reset( );
	}
private:
	static bool IsInstantiated;
	std::unique_ptr< IConsole > mConsole;//TODO: 콘솔을 개발용으로만 둘까, 콘솔에 유저 권한을 둘까?
	///std::unique_ptr< Socket > mSocket;
	std::unordered_map< HashedKey, Dword > mVault;
};

extern ::ServiceLocator gService;