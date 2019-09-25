#include "pch.h"

//TODO: 전역변수와 함수로 모듈화.
//TODO: 소켓 핸들 랜덤이 아닌데?

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

void PrintLeavingE( const size_t population )
{
	std::cout << "A Client left due to an error. (Now " << population << " connections.)\n";
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
	{
		char ipAddress[] = "192.168.219.102";
		if ( -1 == listener.bind(EndPoint(ipAddress, 54321)) )
		{
			listener.close( );
			WSACleanup( );
			std::cerr << "Listener binding failed.\n";
			return -1;
		}
	}
	listener.listen( );
	if ( -1 == iocp.add(listener, nullptr) )
	{
		listener.close( );
		WSACleanup( );
		std::cerr << "Adding listener into ICOP failed.\n";
		return -1;
	}

	auto clientCandidate = std::make_unique<Socket>( Socket::Type::TCP );
	std::string errMsg;
	const int result = listener.acceptOverlapped( *clientCandidate, errMsg );
	if ( 0 == result && ERROR_IO_PENDING != WSAGetLastError( ) )
	{
		listener.close( );
		clientCandidate->close( );
		WSACleanup( );
		std::cerr << "Overlapped acceptEx failed.\n";
		return -1;
	}
	else if ( -1 == result )
	{
		listener.close( );
		clientCandidate->close( );
		WSACleanup( );
		std::cerr << "Getting AcceptEx failed.\n";
		return -1;
	}
	listener.pend( );

	std::cout << "What is today's salt, Sir?" << std::endl;
	char todaysSalt[ 10 ];
	std::cin >> todaysSalt;//궁금: 버퍼 알아서 비워지겠지?
	std::cout << "Ready.\n";
	const uint8_t saltSize = static_cast<uint8_t>(strlen( todaysSalt ));
	const HashedKey refinedSalt = ::util::hash::Digest( todaysSalt, saltSize );
	std::unordered_map< Socket*, std::unique_ptr<Socket> > clientS;
	std::unordered_set< HashedKey > ticketS;
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
					goto cleanUp;
				}
				else
				{
					std::unique_ptr<Socket>& client = clientCandidate;
					if ( -1 == iocp.add(*client, client.get( )) )
					{
						client->close( );
						std::cerr << "Adding client into IOCP failed.\n";
					}
					else
					{
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
#ifdef _DEBUG
							PrintJoining( clientS.size( ) );
#endif
						}
					}
					clientCandidate = std::make_unique<Socket>( Socket::Type::TCP );
					std::string errMsg;
					const int result = listener.acceptOverlapped( *clientCandidate, errMsg );
					if ( 0 != result && ERROR_IO_PENDING != WSAGetLastError( ) )
					{
						// Exception
						std::cerr << "Overlapped acceptEx failed.\n";
						goto cleanUp;
					}
					// Here result is never -1, so this equals 'else if ( 1 == result )'
					else
					{
						listener.pend( );
					}
				}
			}
//			// When event comes from the queue server,
//			else if ( (ULONG_PTR)toQueueServer.get( ) == ev.lpCompletionKey )
//			{
//				switch ( toQueueServer->completedWork( ) )
//				{
//					case ::Socket::CompletedWork::RECEIVE:
//						break;
//					case ::Socket::CompletedWork::SEND:
//						if ( 0 != toQueueServer->receiveOverlapped( )
//							 && ERROR_IO_PENDING != WSAGetLastError( ) )
//						{
//							toQueueServer->close( );
//							toQueueServer.reset( );
//							std::cerr << "Queue server disconnected.\n";
//						}
//						else
//						{
//							toQueueServer->pend( );
//						}
//						break;
//					default:
//#ifdef _DEBUG
//						__debugbreak( );
//#else
//						__assume(0);
//#endif
//
//				}
//			}
			// When event comes from clients,
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
#ifdef _DEBUG
						std::thread th( &PrintLeaving, clientS.size( ) );
						th.detach( );
#endif
					}
					else
					{
						int result = 0;
						switch ( client->completedWork( ) )
						{
							case Socket::CompletedWork::RECEIVE:
							{
								// When event comes from the queue server
								char* rcvBuf = client->receivingBuffer( );
								if ( refinedSalt == ::util::hash::Digest(rcvBuf, saltSize) )
								{
									// When having received just the salt, send the current population.
									if ( '\0' == rcvBuf[saltSize] )
									{
										///toQueueServer = std::move( client );
										std::string population( std::to_string(clientS.size()) );
										result = client->sendOverlapped( population.data(), static_cast<int>(population.size()) );
#ifdef _DEBUG
										std::cout << "Population has been requested and sent: " << population.data( ) << std::endl;
#endif
										///if ( 0 != toQueueServer->sendOverlapped(population.data( ), population.size( ))
										///	 && ERROR_IO_PENDING != WSAGetLastError( ) )
										///{
											///toQueueServer->close( );
											///toQueueServer.reset( );
											///std::cerr << "Queue server disconnected.\n";
										///}
										///else
										///{
											///toQueueServer->pend( );
										///}
										///continue;
									}
									else
									{
										ticketS.emplace( std::atoi(&rcvBuf[saltSize]) );
										result = client->receiveOverlapped( );
#ifdef _DEBUG
										std::cout << "A copy of a ticket arrived: " << &rcvBuf[saltSize] << std::endl;
#endif
									}
								}
								//TODO: 잘못된 salt가 왔을 때
								//else if
								//{}
								// When event comes from ordinary clients
								else
								{
									//TODO
									//result = client->sendOverlapped( client->receivingBuffer( ), ev.dwNumberOfBytesTransferred );
								}
								break;
							}
							case Socket::CompletedWork::SEND:
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
#ifdef _DEBUG
							PrintLeavingE( clientS.size( ) );
#endif
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
cleanUp:
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