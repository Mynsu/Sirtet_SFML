#include "pch.h"

volatile bool IsWorking = true;

void ProcessSignal( int signal )
{
	if ( SIGINT == signal )
	{
		IsWorking = false;
	}
}

void PrintJoining( const size_t population )
{
	std::cout << "A new client joined. (Now " << population << " connections.)\n";
}

void PrintLeaving( const size_t population )
{
	std::cout << "A Client left. (Now " << population << " connections.)\n";
}

int main()
{
	WSAData w;
	WSAStartup( MAKEWORD(2,2), &w );

	signal( SIGINT, &ProcessSignal );

	SYSTEM_INFO sysInfo;
	GetSystemInfo( &sysInfo );
	IOCP iocp( 2*sysInfo.dwNumberOfProcessors + 2 );

	Socket listener( Socket::Type::TCP_LISTENER );
	if ( -1 == listener.bind(EndPoint("192.168.219.102", 54321)) )
	{
		std::cerr << "Listener binding failed.\n";
		return -1;
	}
	listener.listen( );
	if ( -1 == iocp.add(listener, nullptr) )
	{
		std::cerr << "Adding listener into ICOP failed.\n";
		return -1;
	}

	auto clientCandidate = std::make_unique<Socket>( Socket::Type::TCP );
	std::string errMsg;
	const int result = listener.acceptOverlapped( *clientCandidate, errMsg );
	if ( 0 == result && ERROR_IO_PENDING != WSAGetLastError( ) )
	{
		std::cerr << "Overlapped acceptEx failed.\n";
		return -1;
	}
	else if ( -1 == result )
	{
		std::cerr << "Getting AcceptEx failed.\n";
		return -1;
	}
	listener.pend( );

	std::unordered_map< Socket*, std::unique_ptr<Socket> > clientS;
	IOCPEvent event;
	while ( true == IsWorking )
	{
		iocp.wait( event, 100 );
		for ( uint32_t i = 0u; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[ i ];
			// When event comes from listener,
			if ( 0 == ev.lpCompletionKey )
			{
				listener.pend( false );
				if ( 0 != clientCandidate->updateAcceptContext(listener) )//궁금
				{
					// Exception
					listener.close( );
				}
				else
				{
					std::unique_ptr<Socket>& client = clientCandidate;
					if ( -1 == iocp.add(*client, client.get( )) )
					{
						std::cerr << "Adding client into IOCP failed.\n";
						return -1;
					}
					//char data[ ] = "HAHAHA!";
					/*if ( 0 != client->socket( ).sendOverlapped( data, ::strlen( data )+1 )
							&& ERROR_IO_PENDING != WSAGetLastError( ) )*/
					if ( 0 != client->receiveOverlapped( )
						 && ERROR_IO_PENDING != WSAGetLastError( ) )
					{
						// Exception
						client->close( );
					}
					else
					{
						client->pend( );
						clientS.emplace( client.get( ), std::move(client) );
						///std::cout << "A new client joined. (Now " << clientS.size( ) << " connections.)\n";
						std::thread th( &PrintJoining, clientS.size() );
						th.detach();
					}

					clientCandidate = std::make_unique<Socket>( Socket::Type::TCP );
					std::string errMsg;
					const int result = listener.acceptOverlapped( *clientCandidate, errMsg );
					if ( 0 == result && ERROR_IO_PENDING != WSAGetLastError( ) )
					{
						// Exception
						listener.close( );//TODO
						std::cerr << "Overlapped acceptEx failed.\n";
						return -1;
					}
					// Here result is never -1, so this equals 'else if ( 1 == result )'
					else
					{
						listener.pend( );
					}
				}
			}
			// When event comes from TCP
			else
			{
				std::unique_ptr<Socket>& client = clientS[ (Socket*)ev.lpCompletionKey ];
				if ( nullptr != client )
				{
					client->pend( false );
					// 0 in disconnection, <0 in error.
					if ( 0 >= ev.dwNumberOfBytesTransferred )
					{
						client->close( );
						clientS.erase( client.get( ) );
						///std::cout << "A Client left. (Now " << clientS.size( ) << " connections.)\n";
						std::thread th( &PrintLeaving, clientS.size( ) );
						th.detach( );
					}
					else
					{
						// Must make the socket pend by calling 'receive(...)' or 'send(...)'
						// so as to use the overlapped I/O finally.
						int result = 0;
						switch ( client->completedWork( ) )
						{
							case ::Socket::CompletedWork::RECEIVE:
							{
								//char data[] = "HAHAHA!";
								//result = clientSocket.sendOverlapped( data, static_cast<int>(::strlen(data)+1) );
								result = client->sendOverlapped( client->receivingBuffer( ), ev.dwNumberOfBytesTransferred );
								break;
							}
							case ::Socket::CompletedWork::SEND:
								result = client->receiveOverlapped( );
								break;
							default:
#ifdef _DEBUG
								DebugBreak( );
#else
								__assume(0);
#endif
						}

						if ( 0 != result && ERROR_IO_PENDING != WSAGetLastError( ) )
						{
							client->close( );
							clientS.erase( client.get( ) );
							///std::cout << "A Client left. (Now " << clientS.size( ) << " connections.)\n";
							std::thread th( &PrintLeaving, clientS.size( ) );
							th.detach( );
						}
						else
						{
							client->pend( );
						}
					}
				}
			}
		}
	}

	//!IMPORTANT: Must release all the overlapped I/O resources in hand before closing the server.
	//	 		  Otherwise, overlapped I/O still runs on O/S background.
	listener.close( );
	for ( auto& it : clientS )
	{
		it.second->close( );
	}
	std::cout << "Server gets closed.\n";
	while ( 0 < clientS.size( ) || true == listener.isPending( ) )
	{
		// Clear all clients except those still pending at this time.
		#pragma omp parallel
		for ( auto it = clientS.cbegin( ); it != clientS.cend( ); )
		{
			if ( false == it->second->isPending( ) )
			{
				it = clientS.erase( it );
			}
			else
			{
				++it;
			}
		}

		IOCPEvent event;
		iocp.wait( event, 100 );
		for ( uint32_t i = 0; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[ i ];
			// When event comes from listener,
			if ( 0 == ev.lpCompletionKey )
			{
				listener.pend( false );
			}
			// When event comes from TCP,
			else
			{
				std::unique_ptr<Socket>& client = clientS[ (Socket*)ev.lpCompletionKey ];
				if ( nullptr != client )
				{
					client->pend( false );
				}
			}
		}
	}
	std::cout << "Server has been closed." << std::endl;

	WSACleanup( );

	return 0;
}