#pragma once
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32")
#include <stdint.h>
#include "Socket.h"

struct IOCPEvent
{
public:
	IOCPEvent()
		: eventCount( 0 )
	{
		ZeroMemory(events, sizeof(events));
	}
	static const uint16_t MAX_EVENTS = 1000;
	OVERLAPPED_ENTRY events[MAX_EVENTS];
	uint32_t eventCount;
};

class IOCP
{
public:
	IOCP( ) = delete;
	explicit IOCP( uint32_t numOfThreads )
		: mNumOfThreads( numOfThreads ),
		mhIOCP( CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, numOfThreads) )
	{ }
	~IOCP( ) = default;

	int add( const SOCKET_HANDLE handle, const ULONG_PTR id )
	{
		if ( NULL == CreateIoCompletionPort( (HANDLE)handle,
									  mhIOCP,
									  id,
									  mNumOfThreads ) )
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	void wait( IOCPEvent& event, const DWORD timeoutMs )
	{
		if ( FALSE == GetQueuedCompletionStatusEx( mhIOCP, event.events, IOCPEvent::MAX_EVENTS,
											   (PULONG)&event.eventCount,
											   timeoutMs,
											   FALSE ) )
		{
			const int err = WSAGetLastError();
			if ( WSA_WAIT_TIMEOUT == err )
			{
				event.eventCount = 0;
			}
			else
			{
#ifdef _DEBUG
				__debugbreak( );
#else
				std::cerr << "FATAL: GetQueuedCompletionStatusEx(...) returns an error.";
#endif
			}
		}
	}
private:
	const uint32_t mNumOfThreads;
	HANDLE mhIOCP;
};