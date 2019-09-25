#pragma once
#include "Common.h"
#include <string>
#include <MSWSock.h> // LPFN_ACCEPTEX
#pragma comment (lib, "mswsock")
#ifdef _DEBUG
#include <intrin.h>
#else
#include <iostream>
#endif
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

	Socket( )
		: mHasTicket( false ), mIsPending( false ), mCompletedWork( Socket::CompletedWork::RECEIVE ),
		mhSocket( NULL ), AcceptEx( nullptr )
	{
		ZeroMemory( &mOverlappedStruct, sizeof(mOverlappedStruct) );
		ZeroMemory( mRcvBuffer, sizeof(mRcvBuffer) );
		//ZeroMemory( mSndBuffer, sizeof( mSndBuffer ) );//TODO
	}
	Socket( const ::Socket::Type type )
		: Socket( )
	{
		lazyInitialize( type );
	}
	~Socket( )
	{
		mhSocket = NULL;
		AcceptEx = nullptr;
		ZeroMemory( &mOverlappedStruct, sizeof(mOverlappedStruct) );
	}
	void lazyInitialize( const ::Socket::Type type )
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
	}
	int bind( const EndPoint& endpoint )
	{
		if ( NULL == mhSocket )
		{
#ifdef _DEBUG
			__debugbreak();
#else
			std::cerr << "Socket must be initialized before binding.\n";
			return -1;
#endif
		}
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
					  &UUID( WSAID_ACCEPTEX ), sizeof( UUID ), //±Ã±Ý: UUID ÀÌ°Å ¹¹²¿?
					  &AcceptEx, sizeof( AcceptEx ),
					  &bytes, //±Ã±Ý: ÀÌ°Å ¹¹Áö?
					  NULL, NULL );//±Ã±Ý: ÀÌ°Å ¹¹Áö?
			if ( NULL == AcceptEx )
			{
				return -1;
			}
		}
		char acceptedAddress[ 200 ];
		DWORD what = 0;
		if ( 1 == AcceptEx( mhSocket, candidateClientSocket.mhSocket,
							&acceptedAddress,
							0, 50, 50, &what, //±Ã±Ý: ÀÌ°Å ¹¹Áö?
							&mOverlappedStruct ) )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	// ±Ã±Ý: MSDN º¸ÀÚ
	int updateAcceptContext( Socket& listener );
	int receiveOverlapped( );
	int sendOverlapped( char* data, ULONG length );
	void close( )
	{
		closesocket( mhSocket );
	}
	bool hasTicket( ) const
	{
		return mHasTicket;
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
	void earnTicket( )
	{
		mHasTicket = true;
	}
	// Set true while the socket gets ready or waiting for an event, I/O completion.
	// Set false when you touch.
	void pend( const bool isPending = true )
	{
		mIsPending = isPending;
	}
private:
	bool mHasTicket;
	bool mIsPending;
	CompletedWork mCompletedWork;
	SOCKET_HANDLE mhSocket;
	LPFN_ACCEPTEX AcceptEx;
	WSAOVERLAPPED mOverlappedStruct;
	char mRcvBuffer[ MAX_RCV_BUF_LEN ];
	//char mSndBuffer[ MAX_SND_BUF_LEN ];//TODO
};