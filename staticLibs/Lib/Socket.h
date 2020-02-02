#pragma once
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32")
#include <MSWSock.h>
#pragma comment (lib, "mswsock")
#include <forward_list>
#include <string>
#include <memory>
#include <stdint.h>
#include "EndPoint.h"
#include "Hash.h"
#include "Packet.h"

using SOCKET_HANDLE = SOCKET;

enum class IOType
{
	NONE,
	ACCEPT,
	RECEIVE,
	SEND,
	CONNECT,
	DISCONNECT,
};

struct Overlapped : public WSAOVERLAPPED
{
	Overlapped( ) = delete;
	explicit Overlapped( const IOType ioType )
	{
		ZeroMemory( this, sizeof(Overlapped) );
		ioTypeOrIndex = (uint32_t)ioType;
	}
	explicit Overlapped( const uint32_t clientIndex )
	{
		ZeroMemory( this, sizeof(Overlapped) );
		ioTypeOrIndex = clientIndex;
	}
	uint32_t ioTypeOrIndex;
};

class Socket
{
public:
	enum class Type
	{
		TCP,
		UDP,
	};

	static const uint32_t RCV_BUF_SIZ = 8192;
	static const uint32_t BACKLOG_SIZ = 5000;

	Socket( ) = delete;
	Socket( const ::Socket::Type type );
	// !IMPORTANT: DO NOT USE!  Defined to use std::vector.
	Socket( const Socket& )
	{
#ifdef _DEBUG
		__debugbreak( );
#endif
	}
	~Socket( );
	int bind( const EndPoint& selfEndPoint );
	void listen( )
	{
		::listen( mhSocket, BACKLOG_SIZ );
	}
	int acceptOverlapped( Socket& candidateClientSocket, const uint32_t candidateClientIndex );
	int updateAcceptContext( Socket& listener );
	int connect( const EndPoint& targetEndPoint );
	int disconnectOverlapped( );
	int receiveOverlapped( LPWSAOVERLAPPED_COMPLETION_ROUTINE lpRoutine = NULL );
	int receiveBlocking( );
	int sendOverlapped( char* const data, const size_t size,
					   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpRoutine = NULL );
	int sendOverlapped( Packet& packet, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpRoutine = NULL );
	void close( )
	{
		closesocket( mhSocket );
	}
	void reset( const bool isSocketReusable = true, const Socket::Type type = Socket::Type::TCP );
	bool isPending( ) const
	{
		return !mOverlappedStructs.empty();
	}
	SOCKET_HANDLE handle( ) const
	{
		return mhSocket;
	}
	// Returns which IO has been completed.
	// If acceptance has been completed, this returns the index or key of the socket.
	uint32_t completedIO( LPOVERLAPPED const lpOverlapped, const DWORD cbTransferred );
	char* const receivingBuffer( )
	{
		return mReceivingBuffer;
	}
	std::string& extraReceivingBuffer( )
	{
		return mExtraReceivingBuffer;
	}
private:
	void erase( LPOVERLAPPED const lpOverlapped );
	static LPFN_ACCEPTEX AcceptEx;
	static LPFN_DISCONNECTEX DisconnectEx;
	static LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockAddrs;
	bool mIsReceiving_;
	uint32_t mRecentlyReceivedSize_;
	SOCKET_HANDLE mhSocket;
	struct AddressBuffer
	{
		AddressBuffer( )
		{
			ZeroMemory( this, sizeof(AddressBuffer) );
		}
		char localAddress[sizeof(SOCKADDR_IN)+16];
		char remoteAddress[sizeof(SOCKADDR_IN)+16];
	};
	std::unique_ptr<AddressBuffer> mAddressBuffer;
	SOCKADDR_IN* mpRemoteSockAddr;
	std::forward_list<Overlapped> mOverlappedStructs;
	std::string mExtraReceivingBuffer;
	char mReceivingBuffer[ RCV_BUF_SIZ ];
};

// Used as a salt in encrpyting the genuine client's version.
// Recommended to be renewed periodically for security.
#define VERSION 200106
#define SALT 124816