#pragma once
#include <WinSock2.h>
#pragma comment (lib, "Ws2_32")
#include <stdint.h>
#include "Socket.h"

struct IOCPEvent
{
	static const uint32_t MAX_EVENTS = 1000u;
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

	inline int add( const SOCKET_HANDLE handle, const ULONG_PTR id )
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
	inline void wait( IOCPEvent& event, int timeoutMs )
	{
		if ( FALSE == GetQueuedCompletionStatusEx( mhIOCP, event.events, IOCPEvent::MAX_EVENTS,
											   (PULONG)&event.eventCount,
											   timeoutMs,
											   FALSE ) ) //±Ã±Ý: ¹¹Áö?
		{
			const int err = WSAGetLastError();
			if ( WSA_WAIT_TIMEOUT == err )
			{
				event.eventCount = 0;
			}
#ifdef _DEBUG
			else
			{
				__debugbreak( );
			}
#endif
		}
	}
private:
	const uint32_t mNumOfThreads;
	HANDLE mhIOCP;
};