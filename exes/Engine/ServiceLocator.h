#pragma once
#include "pch.h" // Commonly included headers, not pch.
#include "Console.h"

class ServiceLocator
{
public:
	inline ServiceLocator( )
		: mConsole( std::make_unique< ConsoleLocal >( ) )
	{
		ASSERT_FALSE( IsInstantiated );

		WSAData w;
		WSAStartup( MAKEWORD(2, 2), &w );
		mSocket.lazyInitialize( ::Socket::Type::TCP );
		// NOTE: An error occurs.
		///ASSERT_TRUE( -1 != mSocket.bind( EndPoint::Any ) );

		IsInstantiated = true;
	}
	ServiceLocator( const ServiceLocator& ) = delete;
	void operator=( const ServiceLocator& ) = delete;
	inline ~ServiceLocator( )
	{
		mSocket.close();
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
	::Socket& socket( )
	{
		return mSocket;
	}
	void release( )
	{
		// !IMPORTANT: MUST NOT get rid of this line.
		mConsole.reset( );
	}
private:
	static bool IsInstantiated;
	std::unique_ptr< IConsole > mConsole;//TODO: �ܼ��� ���߿����θ� �ѱ�, �ֿܼ� ���� ������ �ѱ�?
	std::unordered_map< HashedKey, Dword > mVault;
	::Socket mSocket;
};

extern ::ServiceLocator gService;