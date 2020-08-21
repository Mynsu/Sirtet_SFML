#pragma once
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment (lib, "Ws2_32")
#include <MSWSock.h>
#pragma comment (lib, "mswsock")
#include <forward_list>
#include <string>
#include <memory>
#include <stdint.h>

using SOCKET_HANDLE = SOCKET;
class EndPoint;
class Packet;

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
	explicit Overlapped( const IOType iotype )
	{
		ZeroMemory( this, sizeof(Overlapped) );
		this->ioType = iotype;
		index = -1;
	}
	explicit Overlapped( const uint16_t clientIndex )
	{
		ZeroMemory( this, sizeof(Overlapped) );
		index = clientIndex;
		ioType = IOType::ACCEPT;
	}
	uint16_t index;
	IOType ioType;
};

class Socket
{
public:
	enum class Type
	{
		TCP,
		UDP,
	};

	static const uint16_t RCV_BUF_SIZ = 8192;

	Socket( ) = delete;
	Socket( const ::Socket::Type type );
	~Socket( );
	// Returns SOCKET_ERROR when failed.
	int bind( const EndPoint& selfEndPoint );
	// Returns SOCKET_ERROR when failed.
	int listen( const uint16_t backlogSize )
	{
		return ::listen( mhSocket, backlogSize );
	}
	// Returns FALSE when failed.
	int acceptOverlapped( Socket& candidateClientSocket, const uint16_t candidateClientIndex );
	// Returns SOCKET_ERROR when failed.
	int updateAcceptContext( Socket& listener );
	// Returns SOCKET_ERROR when failed.
	int connect( const EndPoint& targetEndPoint );
	// Returns FALSE when failed.
	int disconnectOverlapped( );
	// Returns SOCKET_ERROR when failed.
	int receiveOverlapped( LPWSAOVERLAPPED_COMPLETION_ROUTINE lpRoutine = NULL );
	// Returns SOCKET_ERROR when failed.
	int receiveBlocking( );
	// Returns SOCKET_ERROR when failed.
	int sendOverlapped( char* const data, const size_t size,
					   LPWSAOVERLAPPED_COMPLETION_ROUTINE lpRoutine = NULL );
	// Returns SOCKET_ERROR when failed.
	int sendOverlapped( Packet& packet, LPWSAOVERLAPPED_COMPLETION_ROUTINE lpRoutine = NULL );
	// Returns SOCKET_ERROR when failed.
	int close( )
	{
		return ::closesocket( mhSocket );
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
	uint16_t completedIO( LPOVERLAPPED const lpOverlapped, const DWORD cbTransferred );
	char* const receivingBuffer( )
	{
		return mReceivingBuffer;
	}
	std::string& extraReceivingBuffer( )
	{
		return mExtraReceivingBuffer;
	}
	std::string getIPAddress( )
	{
		std::string retVal;
		char hostName[100] = {0};
		if ( 0 != gethostname(hostName, sizeof(hostName)) )
		{
			//std::cerr << "Warning: Failed to get host name(" << WSAGetLastError() << ").\n";
			return retVal;
		}
		addrinfo hint;
		ZeroMemory(&hint, sizeof(hint));
		hint.ai_family = AF_INET;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_protocol = IPPROTO_TCP;
		addrinfo* pResult = nullptr;
		if ( 0 != getaddrinfo(hostName, "0", &hint, &pResult) )
		{
			//std::cerr << "Warning: Failed to get address info(" << WSAGetLastError() << ").\n";
			return retVal;
		}
		for ( addrinfo* pOne = pResult; nullptr != pOne; pOne = pOne->ai_next )
		{
			SOCKADDR_IN* addressInfo = (SOCKADDR_IN*)pOne->ai_addr;
			//char stringBuf[16] = {0};
			char stringBuf[20] = {0};
			retVal += inet_ntop(AF_INET, &addressInfo->sin_addr, stringBuf, sizeof(stringBuf));
			retVal += ", ";
		}
		return retVal;
	}
private:
	void erase( LPOVERLAPPED const lpOverlapped );
	static LPFN_ACCEPTEX AcceptEx;
	static LPFN_DISCONNECTEX DisconnectEx;
	static LPFN_GETACCEPTEXSOCKADDRS GetAcceptExSockAddrs;
	bool mIsReceiving_;
	uint16_t mRecentlyReceivedSize_;
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
	char mReceivingBuffer[RCV_BUF_SIZ];
};