#pragma once
#include "Common.h"
#include <MSWSock.h>
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
		DISCONNECT,
	};

	static const uint32_t MAX_RCV_BUF_LEN = 8192;

	Socket( ) = delete;
	// !IMPORTANT: When replacing an old socket by new one, after having failed disconnecting successfully,
	// you should set the 2nd argument 'work' to Socket::CompletedWork::DISCONNECT.
	// Otherwise, the new socket with the old but same index would act like the old socket did.
	Socket( const ::Socket::Type type, const Socket::CompletedWork work = Socket::CompletedWork::RECEIVE );
	~Socket( );
	int bind( const EndPoint& endpoint );
	void listen( )
	{
		::listen( mhSocket, 5000 );
	}
	int acceptOverlapped( Socket& candidateClientSocket )
	{
		if ( nullptr == AcceptEx )
		{
			DWORD bytes;
			WSAIoctl( mhSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
					  &UUID(WSAID_ACCEPTEX), sizeof(UUID),
					  &AcceptEx, sizeof(AcceptEx),
					  &bytes,
					  NULL, NULL );
			if ( nullptr == AcceptEx )
			{
				return -1;
			}
		}
		char ignored[ 120 ];
		DWORD ignored2 = 0;
		if ( TRUE == AcceptEx( mhSocket, candidateClientSocket.mhSocket,
								&ignored, 0, 50, 50, &ignored2,
								&mOverlappedStruct) )
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	int updateAcceptContext( Socket& listener )
	{
		SOCKADDR_IN ignore1, ignore3;
		INT ignore2, ignore4;
		char ignore[ 120 ];
		GetAcceptExSockaddrs( ignore, 0, 50, 50, (SOCKADDR**)&ignore1, &ignore2,
			(SOCKADDR**)&ignore3, &ignore4 );
		return setsockopt( mhSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
			(char*)&listener.mhSocket, sizeof( listener.mhSocket ) );
	}
	int connect( const EndPoint& endpoint );
	int disconnectOverlapped( )
	{
		if ( nullptr == DisconnectEx )
		{
			DWORD ignored;
			WSAIoctl( mhSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
					  &UUID( WSAID_DISCONNECTEX ), (DWORD)sizeof(UUID),
					  &DisconnectEx, (DWORD)sizeof(DisconnectEx),
					  &ignored, NULL, NULL );
			if ( nullptr == DisconnectEx )
			{
				return -1;
			}
		}
		mCompletedWork = CompletedWork::DISCONNECT;
		return DisconnectEx( mhSocket, &mOverlappedStruct, TF_REUSE_SOCKET, 0 );
	}
	int receiveOverlapped( )
	{
		WSABUF b;
		b.len = MAX_RCV_BUF_LEN;
		b.buf = mRcvBuffer;
		DWORD flag = 0;
		mCompletedWork = CompletedWork::RECEIVE;
		return WSARecv( mhSocket, &b, 1, NULL, &flag, &mOverlappedStruct, NULL );
	}
	int sendOverlapped( char* data, ULONG length )
	{
		WSABUF b;
		// Includes c-style null terminator '\0'.
		b.len = length + 1;
		b.buf = data;
		mCompletedWork = CompletedWork::SEND;
		return ::WSASend( mhSocket, &b, 1, NULL, 0, &mOverlappedStruct, NULL );
	}
	void close( )
	{
		closesocket( mhSocket );
	}
	bool isPending( ) const
	{
		return mIsPending;
	}
	bool hasTicket( ) const
	{
		return mHasTicket;
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
	void earnTicket( bool b = true )
	{
		mHasTicket = b;
	}
private:
	bool mIsPending, mHasTicket;
	CompletedWork mCompletedWork;
	SOCKET_HANDLE mhSocket;
	LPFN_ACCEPTEX AcceptEx;
	LPFN_DISCONNECTEX DisconnectEx;
	WSAOVERLAPPED mOverlappedStruct;
	char mRcvBuffer[ MAX_RCV_BUF_LEN ];
};