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
	std::cout << "A new client wants to join the main server. (Now " << population << " connections remain.)\n";
}

void PrintLeaving( const size_t population )
{
	std::cout << "A Client left. (Now " << population << " connections remain.)\n";
}

void PrintLeavingE( const size_t population )
{
	std::cout << "A Client left due to an error. (Now " << population << " connections remain.)\n";
}

int main( )
{
	WSAData w;
	WSAStartup( MAKEWORD(2, 2), &w );

	signal( SIGINT, &ProcessSignal );
	
	Socket toMainServer( Socket::Type::TCP );
	if ( -1 == toMainServer.bind(EndPoint::Any) )
	{
		toMainServer.close( );
		WSACleanup( );
		std::cerr << "Binding to-main-server socket failed.\n";
		return -1;
	}
	{
		char mainServerIPAddress[] = "192.168.219.102";
		const uint16_t mainServerPort = 54321;
		if ( -1 == toMainServer.connect(EndPoint(mainServerIPAddress,mainServerPort)) )
		{
			toMainServer.close( );
			WSACleanup( );
			std::cerr << "Connection to the main server failed.\n";
			return -1;
		}
		else
		{
			std::cout << "Connection to the main server succeeded.\n";
		}
	}
	///SYSTEM_INFO sysInfo;
	///GetSystemInfo( &sysInfo );
	///IOCP iocp( sysInfo.dwNumberOfProcessors );
	IOCP iocp( 2 );
	if ( -1 == iocp.add(toMainServer, &toMainServer) )
	{
		toMainServer.close( );
		WSACleanup( );
		std::cerr << "Adding to-main-server socket into IOCP failed.\n";
		return -1;
	}
	std::cout << "What is today's salt, Sir?" << std::endl;
	std::string todaysSalt;
	std::cin >> todaysSalt;//궁금: 버퍼 알아서 비워지겠지?
	///const uint8_t saltSize = static_cast<uint8_t>(strlen(todaysSalt));
	// NOTE: Already not pending at first.
	///toMainServer.pend( false );
	if ( 0 != toMainServer.sendOverlapped( todaysSalt.data(), (ULONG)todaysSalt.size() )
		 && ERROR_IO_PENDING != WSAGetLastError( ) )
	{
		toMainServer.close( );
		WSACleanup( );
		std::cerr << "Sending to main server failed.\n";
		return -1;
	}
	else
	{
		toMainServer.pend( );
	}

	Socket listener( Socket::Type::TCP_LISTENER );
	{
		char ipAddress[ ] = "192.168.219.102";
		if ( -1 == listener.bind(EndPoint(ipAddress, 10000)) )
		{
			toMainServer.close( );
			std::cerr << "Listener binding failed.\n";
			IOCPEvent event;
			iocp.wait( event, INFINITE );
			WSACleanup( );
			return -1;
		}
	}
	listener.listen( );
	if ( -1 == iocp.add(listener, NULL) )
	{
		toMainServer.close( );
		listener.close( );
		std::cerr << "Adding listener into ICOP failed.\n";
		IOCPEvent event;
		iocp.wait( event, INFINITE );
		WSACleanup( );
		return -1;
	}
	auto clientCandidate = std::make_unique<Socket>( Socket::Type::TCP );
	std::string errMsg;
	const int result = listener.acceptOverlapped( *clientCandidate, errMsg );
	if ( 0 != result && ERROR_IO_PENDING != WSAGetLastError( ) )
	{
		toMainServer.close( );
		listener.close( );
		clientCandidate->close( );
		std::cerr << "Overlapped acceptEx failed.\n";
		IOCPEvent event;
		iocp.wait( event, INFINITE );
		WSACleanup( );
		return -1;
	}
	else if ( -1 == result )
	{
		toMainServer.close( );
		listener.close( );
		clientCandidate->close( );
		std::cerr << "Getting AcceptEx failed.\n";
		IOCPEvent event;
		iocp.wait( event, INFINITE );
		WSACleanup( );
		return -1;
	}
	listener.pend( );
	std::cout << "Ready.\n";

	const int MAIN_SERVER_CAPACITY = 10u;
	int room = 0;
	bool isAwaitingPopulation = true;
	std::unordered_map< Socket*, std::unique_ptr<Socket> > clientS;
	std::list< Socket* > queue;
	IOCPEvent event;
	while ( true == IsWorking )
	{
		iocp.wait( event, 100 );
		for ( uint32_t i = 0u; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[ i ];
			// When event comes from listener,
			if ( NULL == ev.lpCompletionKey )
			{
				listener.pend( false );
				if ( 0 != clientCandidate->updateAcceptContext(listener) )
				{
					// Exception
					goto cleanUp;
				}
				///else
				///{
					std::unique_ptr<Socket>& client = clientCandidate;
					if ( -1 == iocp.add(*client, client.get()) )
					{
						// Exception
						client->close( );
						std::cerr << "Adding client into IOCP failed.\n";
					}
					else
					{
						// When the main server has enough room for new clients,
						if ( 0 < room )
						{
							const std::string temp( std::to_string(client->handle())+todaysSalt );//궁금: todaysSalt.data()?
							std::string ticket( std::to_string(::util::hash::Digest(temp.data(), static_cast<uint8_t>(temp.size()))) );
							// NOTE: Already not pending at first.
							/// client->pend( false );
							if ( 0 != client->sendOverlapped(ticket.data(), (ULONG)ticket.size())
								 && ERROR_IO_PENDING != WSAGetLastError() )
							{
								client->close( );
								// NOTE: Used as a flag in the branch A below.
								client.reset( );
							}
							else
							{
								client->pend( );
								client->earnTicket( );
								///toMainServer.pend( false );
								if ( 0 != toMainServer.sendOverlapped((todaysSalt+ticket).data(), (ULONG)(todaysSalt.size()+ticket.size()))
									 && ERROR_IO_PENDING != WSAGetLastError() )
								{
									std::cerr << "Sending to main server failed.\n";
									goto cleanUp;
								}
								///else
								///{
									toMainServer.pend( );
									--room;
#ifdef _DEBUG
							std::cout << "Ticket " << ticket.data( ) << " issued.\n";
#endif
								///}
							}
						}
						// When the main server has no room,
						else
						{
							if ( 0 != toMainServer.sendOverlapped(todaysSalt.data(), (ULONG)todaysSalt.size())
								 && ERROR_IO_PENDING != WSAGetLastError( ) )
							{
								std::cerr << "Sending to main server failed.\n";
								goto cleanUp;
							}
							///else
							///{
							toMainServer.pend( );
							isAwaitingPopulation = true;
							///}
							queue.emplace_back( client.get( ) );
							// NOTE: Already not pending at first.
							/// client->pend( false );
							// Not equals to a ticket above.
							std::string queueTicket( "qT:" );
							queueTicket += std::to_string( queue.size() );
							if ( 0 != client->sendOverlapped(queueTicket.data(), (ULONG)queueTicket.size())
								 && ERROR_IO_PENDING != WSAGetLastError( ) )
							{
								client->close( );
								// NOTE: Used as a flag in the branch A below.
								client.reset( );
							}
							else
							{
								client->pend( );
#ifdef _DEBUG
								std::cout << "Queue ticket " << queueTicket.data( ) << " issued.\n";
#endif
							}
						}

						// Branch A
						if ( Socket* const test = client.get( ); nullptr != test )
						{
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
					//else
					//{
						listener.pend( );
					//}
				///}
			}
			// When event comes from the main server
			else if ( (ULONG_PTR)&toMainServer == ev.lpCompletionKey )
			{
				toMainServer.pend( false );
				if ( 0 >= ev.dwNumberOfBytesTransferred )
				{
					std::cerr << "Main server closed or died.\n";
					// Break twice.
					goto cleanUp;
				}
				///else
				///{
					switch( toMainServer.completedWork() )
					{
						case Socket::CompletedWork::RECEIVE:
						{
							room = MAIN_SERVER_CAPACITY - std::atoi(toMainServer.receivingBuffer( ));
#ifdef _DEBUG
							std::cout << "Room in main server: " << room << std::endl;
#endif
							isAwaitingPopulation = false;
							uint32_t deltaQueue = 0;
							///for ( auto const it : queue )
							for ( auto it = queue.cbegin( ); queue.cend( ) != it; )
							{
								// When the main server has enough room for new clients,
								if ( 0 < room )
								{
									std::unique_ptr<Socket>& client = clientS[ *it ];
									const std::string temp( std::to_string(client->handle())+todaysSalt );//궁금
									std::string ticket( std::to_string(::util::hash::Digest(temp.data(), static_cast<uint8_t>(temp.size()))) );
									///client->pend( false );
									if ( 0 != client->sendOverlapped(ticket.data(), (ULONG)ticket.size())
										 && ERROR_IO_PENDING != WSAGetLastError() )
									{
										client->close( );
										clientS.erase( *it );
										it = queue.erase( it );
										++deltaQueue;
#ifdef _DEBUG
										PrintLeavingE( clientS.size( ) );
#endif
									}
									else
									{
										client->pend( );
										client->earnTicket( );
										if ( 0 != toMainServer.sendOverlapped(ticket.data(), (ULONG)ticket.size())
											 && ERROR_IO_PENDING != WSAGetLastError() )
										{
											std::cerr << "Sending to main server failed.\n";
											goto cleanUp;
										}
										///else
										///{
										toMainServer.pend( );
										it = queue.erase( it );
										++deltaQueue;
										--room;
#ifdef _DEBUG
										std::cout << "Ticket " << ticket.data( ) << " issued to someone in queue.\n";
#endif
										///}
									}
								}
								// When the main server has no room,
								else//TODO: 인터벌을 두는 걸로
								{
									if ( 0 != toMainServer.sendOverlapped(todaysSalt.data(), (ULONG)todaysSalt.size())
										 && ERROR_IO_PENDING != WSAGetLastError() )
									{
										std::cerr << "Sending to main server failed.\n";
										goto cleanUp;
									}
									///else
									///{
									toMainServer.pend( );
									isAwaitingPopulation = true;
									///}
									++it;//
									break;
								}
							}

							if ( 0 != deltaQueue )
							{
								for ( auto const it : queue )
								{
									std::unique_ptr<Socket>& client = clientS[ it ];
									std::string queueTicketUpdate( "qTU:-" );
									queueTicketUpdate += std::to_string( deltaQueue );
									if ( 0 != client->sendOverlapped( queueTicketUpdate.data(), (ULONG)queueTicketUpdate.size() )
										 && ERROR_IO_PENDING != WSAGetLastError( ) )
									{
										client->close( );
										clientS.erase( it );
#ifdef _DEBUG
										PrintLeavingE( clientS.size( ) );
#endif
									}
									else
									{
										client->pend( );
#ifdef _DEBUG
										std::cout << "Queue gets short: " << queueTicketUpdate.data( ) << "\n";
#endif
									}
								}
							}
							break;
						}
						case Socket::CompletedWork::SEND:
							if ( true == isAwaitingPopulation )
							{
#ifdef _DEBUG
								std::cout << "Wating for population in server.\n";
#endif
								if ( 0 != toMainServer.receiveOverlapped( )
									 && ERROR_IO_PENDING != WSAGetLastError( ) )
								{
									// Exception
									std::cerr << "Receiving from main server failed.\n";
									goto cleanUp;
								}
								///else
								///{
									toMainServer.pend( );
								///}
							}
							break;
						default:
#ifdef _DEBUG
							__debugbreak( );
#else
							__assume( 0 );
#endif
					}
				///}
			}
			// When event comes from a client
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
						PrintLeaving( clientS.size( ) );
#endif
					}
					else
					{
						switch ( client->completedWork( ) )
						{
							case Socket::CompletedWork::SEND:
								if ( client->hasTicket( ) )
								{
									client->close( );
									clientS.erase( client.get( ) );
#ifdef _DEBUG
									std::cout << "A client left with a ticket. (Now " << clientS.size() << " connections remain.)\n";
#endif
								}
								else
								{
									if ( 0 != client->receiveOverlapped( )
										 && ERROR_IO_PENDING != WSAGetLastError( ) )
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
								break;
							case Socket::CompletedWork::RECEIVE:
								break;
							default:
#ifdef _DEBUG
								__debugbreak( );
#else
								__assume(0);
#endif
						}
					}
				}
			}
		}
	}
cleanUp:
	//!IMPORTANT: Must release all the overlapped I/O resources in hand before closing the server.
	//	 		  Otherwise, overlapped I/O still runs on O/S background.
	toMainServer.close( );
	listener.close( );
	clientCandidate->close( );
	for ( auto& it : clientS )
	{
		it.second->close( );
	}
	std::cout << "Server gets closed.\n";
	while ( 0 < clientS.size() || true == listener.isPending() || true == toMainServer.isPending() )
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
			// When event comes from the main server,
			if ( (ULONG_PTR)&toMainServer == ev.lpCompletionKey )
			{
				toMainServer.pend( false );
			}
			// When event comes from listener,
			else if ( 0 == ev.lpCompletionKey )
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