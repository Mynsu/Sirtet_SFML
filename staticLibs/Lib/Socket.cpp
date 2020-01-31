#pragma hdrstop
#include "Socket.h"
#ifdef _DEBUG
#include <intrin.h>
#else
#include <iostream>
#endif

////
//
////

Work::Work( const IOType ioType )
	: ioType( ioType )
{
	ZeroMemory( &overlapped, sizeof(WSAOVERLAPPED) );
}

void Work::reset()
{
	ioType = IOType::NONE;
	ZeroMemory( &overlapped, sizeof(WSAOVERLAPPED) );
}

////
//
////

LPFN_ACCEPTEX Socket::AcceptEx = nullptr;
LPFN_DISCONNECTEX Socket::DisconnectEx = nullptr;
LPFN_GETACCEPTEXSOCKADDRS Socket::GetAcceptExSockAddrs = nullptr;

Socket::Socket( const ::Socket::Type type )
{
	reset( false, type );
}

Socket::~Socket( )
{
	mpRemoteSockAddr = nullptr;
}

int Socket::bind( const EndPoint& selfEndPoint )
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
	const SOCKADDR_IN& ep = selfEndPoint.get();
	const int result = ::bind(mhSocket, (SOCKADDR*)&ep, sizeof(SOCKADDR_IN));
	return result;
}

int Socket::acceptOverlapped( Socket& candidateClientSocket )
{
	int result = FALSE;
	if ( nullptr == AcceptEx )
	{
		DWORD bytes = 0;
		UUID uuid = WSAID_ACCEPTEX;
		WSAIoctl( mhSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
				 &uuid, sizeof(UUID),
				 &AcceptEx, sizeof(AcceptEx),
				 &bytes,
				 NULL, NULL );
		if ( nullptr == AcceptEx )
		{
			result = -1;
			return result;
		}
	}
	DWORD ignored = 0;
	LPOVERLAPPED lpOverlapped = makeWork(IOType::ACCEPT);
	result = AcceptEx( mhSocket, candidateClientSocket.mhSocket,
					  candidateClientSocket.mAddressBuffer.get(), 0,
					  sizeof(AddressBuffer::localAddress), sizeof(AddressBuffer::remoteAddress),
					  &ignored,
					  lpOverlapped );
	const int err = WSAGetLastError();
	if ( FALSE == result &&
		WSA_IO_PENDING == err )
	{
		result = TRUE;
	}
	return result;
}

int Socket::updateAcceptContext( Socket& listener )
{
	int result = FALSE;
	if ( nullptr == GetAcceptExSockAddrs )
	{
		DWORD bytes = 0;
		UUID uuid = WSAID_GETACCEPTEXSOCKADDRS;
		WSAIoctl( mhSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
				 &uuid, sizeof(UUID),
				 &GetAcceptExSockAddrs, sizeof(GetAcceptExSockAddrs),
				 &bytes, NULL, NULL );
		if ( nullptr == GetAcceptExSockAddrs )
		{
			result = -1;
			return result;
		}
	}

	SOCKADDR_IN* pLocalSockAddr = nullptr;
	INT localSockAddrLength = sizeof(SOCKADDR_IN), remoteSockAddrLength = sizeof(SOCKADDR_IN);
	GetAcceptExSockAddrs( mAddressBuffer.get(), 0,
						 sizeof(AddressBuffer::localAddress), sizeof(AddressBuffer::remoteAddress),
						 (sockaddr**)&pLocalSockAddr, &localSockAddrLength,
						 (sockaddr**)&mpRemoteSockAddr, &remoteSockAddrLength );
	mAddressBuffer.reset( );
	return setsockopt( mhSocket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
		(char*)&listener.mhSocket, sizeof(listener.mhSocket) );
}

int Socket::connect( const EndPoint& targetEndPoint )
{
	const SOCKADDR_IN& ep = targetEndPoint.get();
	const int result = ::connect(mhSocket, (SOCKADDR*)&ep, sizeof(SOCKADDR_IN));
	return result;
}

int Socket::disconnectOverlapped( )
{
	int result = 0;
	if ( nullptr == DisconnectEx )
	{
		DWORD ignored;
		WSAIoctl( mhSocket, SIO_GET_EXTENSION_FUNCTION_POINTER,
				 &UUID( WSAID_DISCONNECTEX ), (DWORD)sizeof(UUID),
				 &DisconnectEx, (DWORD)sizeof(DisconnectEx),
				 &ignored, NULL, NULL );
		if ( nullptr == DisconnectEx )
		{
			result = -1;
			return result;
		}
	}
	LPOVERLAPPED lpOverlapped = makeWork(IOType::DISCONNECT);
	result = DisconnectEx(mhSocket, lpOverlapped, TF_REUSE_SOCKET, 0);
	const int err = WSAGetLastError();
	if ( FALSE == result )
	{
		if ( WSA_IO_PENDING == err )
		{
			result = TRUE;
		}
		else if ( WSAENOTCONN == err )
		{
			result = TRUE;
			for ( Work& w : mWorks )
			{
				if ( lpOverlapped == &w.overlapped )
				{
					w.reset( );
					--mNumOfWorks_;
				}
			}
		}
	}
	return result;
}

int Socket::receiveOverlapped( LPWSAOVERLAPPED_COMPLETION_ROUTINE lpRoutine )
{
	if ( true == mIsReceiving_ )
	{
		return -2;
	}

	ZeroMemory( mReceivingBuffer, mRecentlyReceivedSize_ );
	mExtraReceivingBuffer.clear();
	mExtraReceivingBuffer.resize( 0 );

	WSABUF b;
	b.len = RCV_BUF_SIZ;
	b.buf = mReceivingBuffer;
	DWORD ignored = 0;
	LPOVERLAPPED lpOverlapped = makeWork(IOType::RECEIVE);
	int result = WSARecv( mhSocket, &b, 1, NULL, &ignored, lpOverlapped, lpRoutine );
	const int err = WSAGetLastError();
	if ( 0 == result ||
		-1 == result && WSA_IO_PENDING == err ||
		-1 == result && WSA_IO_INCOMPLETE == err )
	{
		mIsReceiving_ = true;
		result = -2;
	}
	// Exception
	else
	{
		for ( Work& w : mWorks )
		{
			if ( lpOverlapped == &w.overlapped )
			{
				w.reset( );
				--mNumOfWorks_;
				break;
			}
		}
	}
	return result;
}

int Socket::receiveBlocking( )
{
	if ( '\0' != *mReceivingBuffer )
	{
		ZeroMemory( mReceivingBuffer, mRecentlyReceivedSize_ );
	}
	if ( false == mExtraReceivingBuffer.empty() )
	{
		mExtraReceivingBuffer.clear();
		mExtraReceivingBuffer.resize( 0 );
	}

	return ::recv(mhSocket, mReceivingBuffer, (int)RCV_BUF_SIZ, 0 );
}

int Socket::sendOverlapped( char* const data, const size_t size,
						   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpRoutine )
{
	WSABUF b;
	b.buf = data;
	b.len = (ULONG)size;
	LPOVERLAPPED lpOverlapped = makeWork( IOType::SEND );
	int result = ::WSASend( mhSocket, &b, 1, NULL, 0, lpOverlapped, lpRoutine );
	const int err = WSAGetLastError();
	if ( 0 == result ||
		-1 == result && WSA_IO_PENDING == err ||
		-1 == result && WSA_IO_INCOMPLETE == err )
	{
		result = -2;
	}
	// Exception
	else
	{
		for ( Work& w : mWorks )
		{
			if ( lpOverlapped == &w.overlapped )
			{
				w.reset( );
				--mNumOfWorks_;
				break;
			}
		}
	}
	return result;
}

int Socket::sendOverlapped( Packet& packet, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpRoutine )
{
	std::string& data = packet.data();
	WSABUF b;
	b.buf = data.data();
	b.len = (ULONG)data.size();
	LPOVERLAPPED lpOverlapped = makeWork(IOType::SEND);
	int result = ::WSASend( mhSocket, &b, 1, NULL, 0, lpOverlapped, lpRoutine );
	const int err = WSAGetLastError();
	if ( 0 == result ||
		-1 == result &&	WSA_IO_PENDING == err ||
		-1 == result && WSA_IO_INCOMPLETE == err )
	{
		result = -2;
	}
	// Exception
	else
	{
		for ( Work& w : mWorks )
		{
			if ( lpOverlapped == &w.overlapped )
			{
				w.reset( );
				--mNumOfWorks_;
				break;
			}
		}
	}
	return result;
}

void Socket::reset( const bool isSocketReusable, const Socket::Type type )
{
	mIsReceiving_ = false;
	mNumOfWorks_ = 0;
	mRecentlyReceivedSize_ = RCV_BUF_SIZ;
	mAddressBuffer = std::make_unique<AddressBuffer>();
	mpRemoteSockAddr = nullptr;
	for ( Work& w : mWorks )
	{
		w.reset( );
	}
	if ( false == isSocketReusable )
	{
		switch ( type )
		{
			case Socket::Type::TCP:
				mhSocket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				break;
			case Socket::Type::UDP:
				mhSocket = WSASocket(AF_INET, SOCK_DGRAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
				break;
			default:
#ifdef _DEBUG
				__debugbreak( );
#else
				__assume( 0 );
#endif
				break;
		}
	}
}

IOType Socket::completedIO( const LPOVERLAPPED lpOverlapped,
						   const DWORD cbTransferred )
{
	IOType retVal = IOType::NONE;
	for ( Work& w : mWorks )
	{
		if ( lpOverlapped == &w.overlapped )
		{
			retVal = w.ioType;
			w.reset( );
			//
			--mNumOfWorks_;
			if ( IOType::RECEIVE == retVal )
			{
				mIsReceiving_ = false;
				mRecentlyReceivedSize_ = cbTransferred;
			}
			break;
		}
	}
#ifdef _DEBUG
	if ( IOType::NONE == retVal )
	{
		__debugbreak( );
	}
#endif
	return retVal;
}

LPOVERLAPPED Socket::makeWork( const IOType ioType )
{
	LPOVERLAPPED retVal = nullptr;
	for ( Work& w : mWorks )
	{
		// When finding one reusable,
		if ( IOType::NONE == w.ioType )
		{
			++mNumOfWorks_;
			w.ioType = ioType;
			retVal = &w.overlapped;
			return retVal;
		}
	}
	Work& w = mWorks.emplace_front(ioType);
	++mNumOfWorks_;
#ifdef _DEBUG
	if ( -1 == mNumOfWorks_ )
	{
		__debugbreak( );
	}
#endif
	retVal = &w.overlapped;
	return retVal;
}