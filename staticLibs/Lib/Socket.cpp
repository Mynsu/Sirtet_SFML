#pragma hdrstop
#include "Socket.h"
#ifdef _DEBUG
#include <intrin.h>
#else
#include <iostream>
#endif

Socket::Socket( const::Socket::Type type, const Socket::CompletedWork work )
	: mIsPending( false ), mHasTicket( false ),
	mCompletedWork( work ),
	mhSocket( NULL ), AcceptEx( nullptr ), DisconnectEx( nullptr )
{
	switch ( type )
	{
		case ::Socket::Type::UDP:
			mhSocket = WSASocket( AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
			break;
		case ::Socket::Type::TCP:
			[[ fallthrough ]];
		case ::Socket::Type::TCP_LISTENER:
			mhSocket = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
			break;
		default:
			__assume(0);
	}
	ZeroMemory( &mOverlappedStruct, sizeof( mOverlappedStruct ) );
	ZeroMemory( mRcvBuffer, sizeof( mRcvBuffer ) );
}

Socket::~Socket( )
{
	mhSocket = NULL;
	AcceptEx = nullptr;
	DisconnectEx = nullptr;
}

int Socket::bind( const EndPoint& endpoint )
{
	if ( NULL == mhSocket )
	{
#ifdef _DEBUG
		__debugbreak( );
#else
		std::cerr << "Socket must be initialized before binding.\n";
		return -1;
#endif
	}
	const SOCKADDR_IN ep = endpoint.get( );
	int retVal = ::bind( mhSocket, (SOCKADDR*)&ep, sizeof( decltype(endpoint.get( )) ) );
	return retVal;
}

int Socket::connect( const EndPoint& endpoint )
{
	const SOCKADDR_IN ep = endpoint.get( );
	int retVal = ::connect( mhSocket, (SOCKADDR*)&ep, sizeof( decltype(endpoint.get( )) ) );
	return retVal;
}

int Socket::receiveBlock( )
{
	return ::recv(mhSocket, mRcvBuffer, (int)RCV_BUF_SIZ, 0 );
}