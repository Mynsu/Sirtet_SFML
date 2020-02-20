#pragma once
#include "pch.h" // Commonly included headers, not pch.
#include "Console.h"
#include "Sound.h"

class ServiceLocator final : public IServiceLocator
{
public:
	ServiceLocator( )
		: mSound( std::make_unique<Sound>() )
	{
		ASSERT_TRUE( false == IsInstantiated );

		WSAData w;
		WSAStartup( MAKEWORD(2, 2), &w );

		IsInstantiated = true;
	}
	ServiceLocator( const ServiceLocator& ) = delete;
	void operator=( const ServiceLocator& ) = delete;
	ServiceLocator( ServiceLocator&& ) = delete;
	~ServiceLocator( )
	{
		WSACleanup();

		IsInstantiated = false;
	}

	// Access the console.  For API.
	IConsole& console( ) override
	{
		return mConsole;
	}
	// Access the console.  For internal use.
	Console& _console( )
	{
		return mConsole;
	}
	// Access global variables.
	auto vault( ) -> std::unordered_map<HashedKey, Dword>&
	{
		return mVault;
	}
	void registerSound( std::unique_ptr<ISound>& soundService )
	{
		mSound = std::move(soundService);
	}
	auto sound( ) -> ISound&
	{
		return *mSound;
	}
	void release( )
	{
		// !IMPORTANT: MUST NOT get rid of this line.
		mConsole.release( );
	}
private:
	static bool IsInstantiated;
	std::unique_ptr<ISound> mSound;
	Console mConsole;
	std::unordered_map<HashedKey, Dword> mVault;
};

extern ::ServiceLocator gService;