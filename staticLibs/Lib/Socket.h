#pragma once
#include "Common.h"
#include <MSWSock.h>
#pragma comment (lib, "mswsock")
#include <string>
#include "EndPoint.h"
#include "Hash.h"

using SOCKET_HANDLE = SOCKET;
constexpr char TOKEN_SEPARATOR = ' ';
constexpr char TOKEN_SEPARATOR_2 = '_';

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
		NONE,
	};

	static const uint32_t RCV_BUF_SIZ = 8192;

	Socket( ) = delete;
	// !IMPORTANT: When replacing an old socket by new one,
	// after having failed disconnecting successfully,
	// you should set the 2nd argument 'work' to Socket::CompletedWork::DISCONNECT.
	// Otherwise, the new socket with the old but same index would act like the old socket did.
	Socket( const ::Socket::Type type,
		   const Socket::CompletedWork work = Socket::CompletedWork::NONE );
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
		b.len = RCV_BUF_SIZ;
		b.buf = mRcvBuffer;
		DWORD flag = 0;
		mCompletedWork = CompletedWork::RECEIVE;
		const int res = WSARecv( mhSocket, &b, 1, NULL, &flag, &mOverlappedStruct, NULL );
		return (-1==res&&ERROR_IO_PENDING==WSAGetLastError())? -2: res;
	}
	int receiveBlock( );
	int sendOverlapped( char* const data, const size_t size,
					   const char separator = TOKEN_SEPARATOR )
	{
		WSABUF b;
		int res = -2;
		if ( separator != data[size-1u] )
		{
			std::string _data( data, size+1u );
			_data[size] = separator;
			b.buf = _data.data( );
			b.len = (ULONG)_data.size( );
			res = ::WSASend( mhSocket, &b, 1, NULL, 0, &mOverlappedStruct, NULL );
		}
		else
		{
			b.buf = data;
			b.len = (ULONG)size;
			res = ::WSASend( mhSocket, &b, 1, NULL, 0, &mOverlappedStruct, NULL );
		}
		mCompletedWork = CompletedWork::SEND;
		return (-1==res&&ERROR_IO_PENDING==WSAGetLastError())? -2: res;
	}
	int sendBlock( char* const data, const int size,
				  const char separator = TOKEN_SEPARATOR );

	void close( )
	{
		closesocket( mhSocket );
	}
	bool isPending( ) const
	{
		return mIsPending;
	}
//TODO
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
//TODO
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
	char mRcvBuffer[ RCV_BUF_SIZ ];
};

// Used as a salt in encrpyting the genuine client's version.
// Recommended to be renewed periodically for security.
const int ADDITIVE = 1246;

using Tag = char[];

// The queue server sends encrypted data attached by this tag to the both of them,
// the main server and ...
// ... the client having submitted the valid invitation.
// The client receives and sends to the main server as it is.
// With that the main server can see the connecting client is genuine or not.
constexpr Tag TAG_TICKET = "t:";
constexpr uint8_t TAG_TICKET_LEN = ::util::hash::Measure( TAG_TICKET );

// The queue server sends the client's order attached by this tag.
constexpr Tag TAG_ORDER_IN_QUEUE = "qL:";
constexpr uint8_t TAG_ORDER_IN_QUEUE_LEN = ::util::hash::Measure( TAG_ORDER_IN_QUEUE );

constexpr Tag TAG_NICKNAME = "nck:";
constexpr uint8_t TAG_NICKNAME_LEN = ::util::hash::Measure( TAG_NICKNAME );

//constexpr Tag TAG_NOTIFY_JOINING = "nJ:";
//constexpr uint8_t TAG_NOTIFY_JOINING_LEN = ::util::hash::Measure( TAG_NOTIFY_JOINING );
//
//constexpr Tag TAG_OLDERS = "old:";
//constexpr uint8_t TAG_OLDERS_LEN = ::util::hash::Measure( TAG_OLDERS );

enum class Request
{
	CREATE_ROOM,
	START_GAME,
	GET_READY,
	INVITE,
};

constexpr char RESPONSE_AFFIRMATION = '1';
constexpr char RESPONSE_NEGATION = '0';

constexpr char _TAG_CREATE_ROOM = '0' + (int)Request::CREATE_ROOM;
constexpr char TAG_CREATE_ROOM[] = { _TAG_CREATE_ROOM, ':', '\0' };

constexpr char _TAG_START_GAME = '0' + (int)Request::START_GAME;
constexpr char TAG_START_GAME[ ] = { _TAG_START_GAME, ':', '\0' };

constexpr char _TAG_GET_READY = '0' + (int)Request::GET_READY;
constexpr char TAG_GET_READY[ ] = { _TAG_GET_READY, ':', '\0' };

//constexpr char _TAG_INVITE = '0' + (int)Request::INVITE;
//constexpr char TAG_INVITE[] = { _TAG_INVITE, ':', '\0' };

////
// inPlay
////

constexpr char TAG_CURRENT_TETRIMINO[ ] = "curT:";