#pragma once
#include "Common.h"
#include "Socket.h"

struct IOCPEvent
{
	static const unsigned MAX_EVENTS = 1000u;
	OVERLAPPED_ENTRY events[ MAX_EVENTS ];
	int eventCount;
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

	inline int add( Socket& socket, ULONG_PTR id )
	{
		if ( NULL == CreateIoCompletionPort( reinterpret_cast<HANDLE>(socket.handle( )),
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
		if ( 0 == GetQueuedCompletionStatusEx( mhIOCP, event.events, IOCPEvent::MAX_EVENTS,
											   reinterpret_cast<PULONG>(&event.eventCount),
											   timeoutMs,
											   FALSE ) ) //±Ã±Ý: ¹¹Áö?
		{
			// When it does nothing,
			event.eventCount = 0;
		}
	}
private:
	const uint32_t mNumOfThreads;
	HANDLE mhIOCP;
};