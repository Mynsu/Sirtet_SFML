#pragma once
#include <memory>
#include <Lib/Hash.h>
#include <Lib/Socket.h>
#include "Console.h"

class ServiceLocator
{
public:
	inline ServiceLocator( )
		: mConsole( std::make_unique< ConsoleLocal >( ) ), mSocket( ::Socket::Type::TCP )
	{
		WSAData w;
		WSAStartup( MAKEWORD( 2, 2 ), &w );
	}
	ServiceLocator( const ServiceLocator& ) = delete;
	void operator=( const ServiceLocator& ) = delete;
	inline ~ServiceLocator( )
	{
		mSocket.close();
		WSACleanup();
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
		mConsole.reset( );
	}
private:
	std::unique_ptr< IConsole > mConsole;//TODO: �ܼ��� ���߿����θ� �ѱ�, �ֿܼ� ���� ������ �ѱ�?
	std::unordered_map< HashedKey, Dword > mVault;
	::Socket mSocket;
};

extern ::ServiceLocator gService;