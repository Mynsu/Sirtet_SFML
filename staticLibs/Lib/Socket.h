#pragma once
#include "Common.h"
#include <string>
#include <MSWSock.h> // LPFN_ACCEPTEX
#pragma comment (lib, "mswsock")
#include "EndPoint.h"

using SOCKET_HANDLE = SOCKET;

class Socket
{
public:
	enum class Type
	{
		TCP_LISTENER,
		TCP,
		UDP,
	};

	enum class CompletedWork
	{
		RECEIVE,
		SEND,
	};

	static const uint32_t MAX_RCV_BUF_LEN = 8192;
	//static const uint32_t MAX_SND_BUF_LEN = 1024;//TODO

	Socket( ) = delete;
	Socket( const Socket::Type type )
		: mIsPending( false ), AcceptEx( nullptr )
	{
		switch ( type )
		{
			case Socket::Type::UDP:
				mhSocket = WSASocket( AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );//±√±›: pch∂ßπÆø° ¿Œ∂Û¿Œ æ» µ«¡ˆ æ ¿ª±Ó?
				break;
			case Socket::Type::TCP:
				[[ fallthrough ]];
			case Socket::Type::TCP_LISTENER:
				mhSocket = WSASocket( AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED );
				break;
			default:
				__assume(0);
		}
		ZeroMemory( &mOverlappedStruct, sizeof(mOverlappedStruct) );
		ZeroMemory( mRcvBuffer, sizeof(mRcvBuffer) );
		//ZeroMemory( mSndBuffer, sizeof( mSndBuffer ) );//TODO
	}
	~Socket( )
	{
		mhSocket = NULL;
		AcceptEx = nullptr;
		ZeroMemory( &mOverlappedStruct, sizeof(mOverlappedStruct) );
	}
	int bind( const EndPoint& endpoint )
	{
		const SOCKADDR_IN ep = endpoint.get( );
		int retVal = ::bind( mhSocket, (SOCKADDR*)&ep, sizeof( decltype(endpoint.get( )) ) );
		return retVal;
	}
	inline void listen( )
	{
		::listen( mhSocket, 5000 );
	}
	int connect( const EndPoint& endpoint )
	{
		const SOCKADDR_IN ep = endpoint.get( );
		int retVal = ::connect( mhSocket, (SOCKADDR*)&ep, sizeof( decltype(endpoint.get( )) ) );
		return retVal;
	}
	inline int acceptOverlapped( Socket& candidateClientSocket, std::string& errorMessage )
	{
		if ( nullptr == AcceptEx )
		{
			DWORD bytes;
			WSAIoctl( mhSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
					  &UUID( WSAID_ACCEPTEX ), sizeof( UUID ), //±√±›: UUID ¿Ã∞≈ ππ≤ø?
					  &AcceptEx, sizeof( AcceptEx ),
					  &bytes, //±√±›: ¿Ã∞≈ ππ¡ˆ?
					  NULL, NULL );//±√±›: ¿Ã∞≈ ππ¡ˆ?
			if ( NULL == AcceptEx )
			{
				return -1;
			}
		}
		char acceptedAddress[ 200 ];
		DWORD what = 0;
		if ( 1 == AcceptEx( mhSocket, candidateClientSocket.mhSocket,
							&acceptedAddress,
							0, 50, 50, &what, //±√±›: ¿Ã∞≈ ππ¡ˆ?
							&mOverlappedStruct ) )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	// ±√±›: MSDN ∫∏¿⁄
	int updateAcceptContext( Socket& listener );
	int receiveOverlapped( );
	int sendOverlapped( char* data, ULONG length );
	void close( )
	{
		closesocket( mhSocket );
	}
	bool isPending( ) const
	{
		return mIsPending;
	}
	CompletedWork completedWork( ) const
	{
		return mCompletedWork;
	}
	SOCKET_HANDLE handle( ) const
	{
		return mhSocket;
	}
	char* const receivingBuffer( )
	{
		return mRcvBuffer;
	}
	// Set true while the socket gets ready or waiting for an event, I/O completion.
	// Set false when you touch.
	void pend( const bool isPending = true )
	{
		mIsPending = isPending;
	}
private:
	bool mIsPending;
	CompletedWork mCompletedWork;
	SOCKET_HANDLE mhSocket;
	LPFN_ACCEPTEX AcceptEx;
	WSAOVERLAPPED mOverlappedStruct;
	char mRcvBuffer[ MAX_RCV_BUF_LEN ];
	//char mSndBuffer[ MAX_SND_BUF_LEN ];//TODO
};