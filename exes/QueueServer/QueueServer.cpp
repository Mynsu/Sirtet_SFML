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
const uint32_t CAPACITY = 1u;
void PrintLeavingE( const size_t population )
{
	std::cout << "A client left due to an error. (Now " << population << '/' << CAPACITY << " connections.)\n";
}
int ConnectToMainServer( std::unique_ptr<Socket>& toMainServer )
{
	char mainServerIPAddress[ ] = "192.168.219.102";
	const uint16_t mainServerPort = 54321;
	int tolerance = 0;
	while ( -1 == toMainServer->connect(EndPoint(mainServerIPAddress, mainServerPort)) )
	{
		// Exception
		std::cerr << "Connection to the main server failed.\n";
		++tolerance;
		if ( 3 == tolerance )
		{
			return -1;
		}
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
	}
	std::cout << "Connection to the main server succeeded.\n";

	return 0;
}
int ResetMainServerSocket( std::unique_ptr<Socket>& toMainServer )
{
	toMainServer->close( );
	toMainServer = std::make_unique<Socket>( Socket::Type::TCP );
	if ( -1 == toMainServer->bind(EndPoint::Any) )
	{
		// Exception
		std::cerr << "Binding to-main-server socket failed.\n";
		return -1;
	}
	return ConnectToMainServer( toMainServer );
}

int main( )
{
	WSAData w;
	WSAStartup( MAKEWORD(2, 2), &w );

	signal( SIGINT, &ProcessSignal );
	
	std::unique_ptr< Socket > toMainServer( std::make_unique<Socket>(Socket::Type::TCP) );
	if ( -1 == toMainServer->bind(EndPoint::Any) )
	{
		// Exception
		std::cerr << "Binding to-main-server socket failed.\n";
		toMainServer->close( );
		WSACleanup( );
		return -1;
	}
	if ( -1 == ConnectToMainServer(toMainServer) )
	{
		// Exception
		toMainServer->close( );
		WSACleanup( );
		return -1;
	}

	///SYSTEM_INFO sysInfo;
	///GetSystemInfo( &sysInfo );
	///IOCP iocp( sysInfo.dwNumberOfProcessors );
	
	using index = uint32_t;
	const index MAIN_SERV_IDX = CAPACITY;
	IOCP iocp( 2 );
	std::cout << "\n##########\n### QUEUE SERVER\n##########\n\nWhat is today's salt, Sir?" << std::endl;
	std::string todaysSalt;
	std::cin >> todaysSalt;
	// NOTE: Skipped because a socket isn't pending at first.
	///toMainServer.pend( false );
	int tolerance = 0;
	if ( -1 == iocp.add(*toMainServer, MAIN_SERV_IDX) )
	{
		// Exception
		toMainServer->close( );
		WSACleanup( );
		std::cerr << "Adding to-main-server socket into IOCP failed.\n";
		return -1;
	}
	while ( -1 == toMainServer->sendOverlapped(todaysSalt.data(), (ULONG)todaysSalt.size())
		 && ERROR_IO_PENDING != WSAGetLastError( ) )
	{
		// Exception
		std::cerr << "Sending to main server failed.\n";
		++tolerance;
		if ( 3 == tolerance || -1 == ResetMainServerSocket(toMainServer) )
		{
			toMainServer->close( );
			WSACleanup( );
			return -1;
		}
	}
	toMainServer->pend( );

	Socket listener( Socket::Type::TCP_LISTENER );
	{
		char ipAddress[ ] = "192.168.219.102";
		if ( -1 == listener.bind(EndPoint(ipAddress, 10000)) )
		{
			// Exception
			toMainServer->close( );
			std::cerr << "Listener binding failed.\n";
			IOCPEvent event;
			iocp.wait( event, INFINITE );
			WSACleanup( );
			return -1;
		}
	}
	listener.listen( );
	const index LISTENER_IDX = CAPACITY + 1u;
	if ( -1 == iocp.add(listener, LISTENER_IDX) )
	{
		// Exception
		toMainServer->close( );
		listener.close( );
		std::cerr << "Adding listener into ICOP failed.\n";
		IOCPEvent event;
		iocp.wait( event, INFINITE );
		WSACleanup( );
		return -1;
	}
	std::vector< Socket > clientS;
	clientS.reserve( CAPACITY );
	// Must use push_back(or emplace_back), front and pop_front only, like std::queue.
	std::list< index > candidateS;
	for ( uint32_t i = 0u; i != CAPACITY; ++i )
	{
		clientS.emplace_back( Socket::Type::TCP );
		if ( -1 == iocp.add(clientS.at(i), i) )
		{
			toMainServer->close( );
			listener.close( );
			IOCPEvent event;
			iocp.wait( event, INFINITE );
			WSACleanup( );
			clientS.clear( );
			break;
		}
		candidateS.emplace_back( i );
	}
	const int result = listener.acceptOverlapped( clientS.at(candidateS.front()) );
	if ( 0 == result && ERROR_IO_PENDING != WSAGetLastError( ) )
	{
		// Exception
		std::cerr << "Overlapped acceptEx failed.\n";
		IsWorking = false;
	}
	else if ( -1 == result )
	{
		// Exception
		std::cerr << "Getting AcceptEx failed.\n";
		IsWorking = false;
	}
	listener.pend( );
	std::cout << "Ready.\n";

	// Capacity in the main server defaults to 10000u.
	// You can resize it indirectly here without re-compliing or resizing directly the server.
	// !IMPORTANT: This must be less than the capacity of the server.
	const uint32_t MAIN_SERVER_CAPACITY = CAPACITY;
	int room = 0;
	bool isAwaitingPopulation = true;
	bool wasBoatful = false;
	std::list< index > queue;
	IOCPEvent event;
	while ( true == IsWorking )
	{
		iocp.wait( event, 100 );
		for ( uint32_t i = 0u; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[ i ];
			// When event comes from listener,
			if ( LISTENER_IDX == (index)ev.lpCompletionKey && 0 < candidateS.size() )
			{
				listener.pend( false );
				const index candidateClientIdx = candidateS.front( );
				Socket& candidateSocket = clientS[ candidateClientIdx ];
				if ( -1 == candidateSocket.updateAcceptContext(listener) )
				{
					// Exception
					// NOTE: Break and break
					goto cleanUp;
				}
				
				// When acception is successful,
				candidateS.pop_front( );
#ifdef _DEBUG
				std::cout << "A new client wants to join the main server. (Now " << CAPACITY-candidateS.size() << "/" << CAPACITY << " connections.)\n";
#endif
				Socket& clientSocket = candidateSocket;
				const index clientIdx = candidateClientIdx;
				// When the main server has enough room for new clients,
				if ( 0 < room )
				{
					const std::string ticketNum( std::to_string(clientSocket.handle())+todaysSalt );
					std::string ticket( std::to_string(::util::hash::Digest(ticketNum.data(), static_cast<uint8_t>(ticketNum.size()))) );
					// NOTE: Skipped because a socket isn't pending at first.
					/// client->pend( false );
					if ( -1 == clientSocket.sendOverlapped(ticket.data(), (ULONG)ticket.size())
						&& ERROR_IO_PENDING != WSAGetLastError() )
					{
						// Exception
						std::cerr << "Sending a ticket to client failed.\n";
						clientSocket.close( );
						clientS[ clientIdx ] = Socket( Socket::Type::TCP );
						if ( -1 == iocp.add(clientS[clientIdx], clientIdx) )
						{
							// Exception
							// Break twice.
							goto cleanUp;
						}
						candidateS.emplace_back( clientIdx );
						PrintLeavingE( CAPACITY - candidateS.size( ) );
					}
					else
					{
						clientSocket.pend( );
						clientSocket.earnTicket( );
						toMainServer->pend( false );
						tolerance = 0;
						while ( -1 == toMainServer->sendOverlapped(ticket.data(), (ULONG)ticket.size())
								&& ERROR_IO_PENDING != WSAGetLastError() )
						{
							// Exception
							std::cerr << "Sending the issued ticket to main server failed.\n";
							++tolerance;
							if ( 3 == tolerance || -1 == ResetMainServerSocket(toMainServer)
								 || -1 == iocp.add( *toMainServer, MAIN_SERV_IDX ) )
							{
								// Exception
								// Break twice.
								goto cleanUp;
							}
						}
						toMainServer->pend( );
						--room;
#ifdef _DEBUG
						std::cout << "Ticket " << ticket.data( ) << " issued successfully.\n";
#endif
					}
				}
				// When the main server has no room,
				else
				{
					tolerance = 0;
					while ( -1 == toMainServer->sendOverlapped(todaysSalt.data(), (ULONG)todaysSalt.size())
							&& ERROR_IO_PENDING != WSAGetLastError( ) )
					{
						// Exception
						std::cerr << "Asking how much room remains in the main server failed.\n";
						++tolerance;
						if ( 3 == tolerance || -1 == ResetMainServerSocket(toMainServer)
							 || -1 == iocp.add(*toMainServer, MAIN_SERV_IDX) )
						{
							// Exception
							// Break twice.
							goto cleanUp;
						}
					}
					toMainServer->pend( );
					isAwaitingPopulation = true;
					queue.emplace_back( clientIdx );
					// NOTE: Skipped because a socket isn't pending at first.
					/// client->pend( false );
					std::string queueTicket( "qT:" );
					const uint32_t queueTicketNumber = static_cast< uint32_t >( queue.size() );
					queueTicket += std::to_string( queueTicketNumber );
					if ( -1 == clientSocket.sendOverlapped(queueTicket.data(), (ULONG)queueTicket.size())
						&& ERROR_IO_PENDING != WSAGetLastError( ) )
					{
						// Exception
						std::cerr << "Sending a queue ticket to a client failed.\n";
						clientSocket.close( );
						clientS[ clientIdx ] = Socket( Socket::Type::TCP );
						if ( -1 == iocp.add(clientS[clientIdx], clientIdx) )
						{
							// Exception
							// Break twice
							goto cleanUp;
						}
						candidateS.emplace_back( clientIdx );
						PrintLeavingE( CAPACITY - candidateS.size( ) );
					}
					else
					{
						clientSocket.pend( );
#ifdef _DEBUG
						std::cout << "Queue ticket " << queueTicket.data( ) << " issued successfully.\n";
#endif
					}
				}

				// Reloading the next candidate.
				if ( 0 < candidateS.size() )
				{
					const index nextCandidateClientIdx = candidateS.front( );
					if ( 0 == listener.acceptOverlapped(clientS[nextCandidateClientIdx])
						 && ERROR_IO_PENDING != WSAGetLastError( ) )
					{
						// Exception
						std::cerr << "Overlapped acceptEx failed.\n";
						goto cleanUp;
					}
					listener.pend( );
				}
				else
				{
					wasBoatful = true;
				}
			}
			// When event comes from the main server,
			else if ( MAIN_SERV_IDX == (index)ev.lpCompletionKey )
			{
				toMainServer->pend( false );
				if ( 0 >= ev.dwNumberOfBytesTransferred )
				{
					std::cerr << "Main server closed or died.\n";
					if ( -1 == ResetMainServerSocket(toMainServer) )
					{
						// Exception
						// NOTE: Brake twice.
						goto cleanUp;
					}
				}
				else
				{
					switch( toMainServer->completedWork() )
					{
						case Socket::CompletedWork::RECEIVE:
						{
							isAwaitingPopulation = false;
							room = MAIN_SERVER_CAPACITY - std::atoi(toMainServer->receivingBuffer( ));
#ifdef _DEBUG
							std::cout << "Room in the main server: " << room << std::endl;
#endif
							uint32_t deltaQueue = 0;
							for ( auto it = queue.cbegin( ); queue.cend( ) != it; )
							{
								// When room in the main server remains,
								if ( 0 < room )
								{
									Socket& clientSocket = clientS[ *it ];
									const std::string ticketNum( std::to_string(clientSocket.handle())+todaysSalt );
									std::string ticket( std::to_string(::util::hash::Digest(ticketNum.data(), static_cast<uint8_t>(ticketNum.size()))) );
#ifdef _DEBUG
									if ( false != clientSocket.isPending( ) ) __debugbreak( );
#endif
									clientSocket.pend( false );
									if ( -1 == clientSocket.sendOverlapped(ticket.data(), (ULONG)ticket.size())
										&& ERROR_IO_PENDING != WSAGetLastError() )
									{
										// Exception
										std::cerr << "Sending a ticket to a client in queue failed.\n";
										clientSocket.close( );
										clientS[*it] = Socket( Socket::Type::TCP );
										if ( -1 == iocp.add(clientS[*it], *it) )
										{
											// Exception
											// Break twice
											goto cleanUp;
										}
										//TODO
										candidateS.emplace_back( *it );
										PrintLeavingE( CAPACITY - candidateS.size( ) );
									}
									else
									{
										clientSocket.pend( );
										clientSocket.earnTicket( );
										tolerance = 0;
										while ( -1 == toMainServer->sendOverlapped(ticket.data(), (ULONG)ticket.size())
												&& ERROR_IO_PENDING != WSAGetLastError( ) )
										{
											// Exception
											std::cerr << "Sending the issued ticket to main server failed.\n";
											++tolerance;
											if ( 3 == tolerance || -1 == ResetMainServerSocket(toMainServer)
												|| -1 == iocp.add(*toMainServer, MAIN_SERV_IDX) )
											{
												// Exception
												// Break twice.
												goto cleanUp;
											}
										}
										toMainServer->pend( );
										--room;
#ifdef _DEBUG
										std::cout << "Ticket " << ticket.data( ) << " issued to someone in queue.\n";
#endif
									}
									++deltaQueue;
									it = queue.erase( it );
								}
								// When there's no more room in the main server,
								else//TODO: 인터벌을 두는 걸로
								{
									tolerance = 0;
									while ( -1 == toMainServer->sendOverlapped(todaysSalt.data(), (ULONG)todaysSalt.size())
											&& ERROR_IO_PENDING != WSAGetLastError( ) )
									{
										// Exception
										std::cerr << "Asking how much room remains in the main server failed.\n";
										++tolerance;
										if ( 3 == tolerance || -1 == ResetMainServerSocket(toMainServer)
											|| -1 == iocp.add(*toMainServer, MAIN_SERV_IDX) )
										{
											// Exception
											// Break twice.
											goto cleanUp;
										}
									}
									toMainServer->pend( );
									isAwaitingPopulation = true;
									break;
								}
							}

							// Notifying queue(wating line) number update.
							if ( 0 != deltaQueue )
							{
								for ( auto it = queue.cbegin(); queue.cend() != it; )
								{
									Socket& clientSocket = clientS[ *it ];
									std::string queueTicketUpdate( "qTU:-" );
									queueTicketUpdate += std::to_string( deltaQueue );
#ifdef _DEBUG
									if ( false != clientSocket.isPending( ) ) __debugbreak( );
#endif
									clientSocket.pend( false );
									if ( -1 == clientSocket.sendOverlapped(queueTicketUpdate.data(), (ULONG)queueTicketUpdate.size())
										 && ERROR_IO_PENDING != WSAGetLastError( ) )
									{
										// Exception
										std::cerr << "Sending how long the queue get shorter failed.\n";
										clientSocket.close( );
										clientS[ *it ] = Socket( Socket::Type::TCP );
										if ( -1 == iocp.add(clientS[*it], *it) )
										{
											// Exception
											// Break twice
											goto cleanUp;
										}
										candidateS.emplace_back( *it );
										++deltaQueue;
										it = queue.erase( it );
										PrintLeavingE( CAPACITY - candidateS.size() );
									}
									else
									{
										clientSocket.pend( );
										++it;
#ifdef _DEBUG
										std::cout << "Queue gets short by" << queueTicketUpdate.data( ) << "\n";
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
								std::cout << "Wating for population in the main server.\n";
#endif
								tolerance = 0;
								while ( -1 == toMainServer->receiveOverlapped()
										&& ERROR_IO_PENDING != WSAGetLastError() )
								{
									// Exception
									std::cerr << "Receiving from the main server failed.\n";
									++tolerance;
									if ( 3 == tolerance || -1 == ResetMainServerSocket(toMainServer)
										|| -1 == iocp.add(*toMainServer, MAIN_SERV_IDX) )
									{
										// Exception
										// Break twice.
										goto cleanUp;
									}
								}
								toMainServer->pend( );
							}
							break;
						default:
#ifdef _DEBUG
							__debugbreak( );
#else
							__assume( 0 );
#endif
					}
				}
			}
			// When event comes from a client
			else
			{
				const index clientIdx = (index)ev.lpCompletionKey;
				Socket& clientSocket = clientS[ clientIdx ];
				clientSocket.pend( false );
				if ( Socket::CompletedWork::DISCONNECT == clientSocket.completedWork( ) )
				{
					//TODO
					//candidateS.emplace_back( clientIdx );
				}
				// 0 in disconnection, <0 in error.
				else if ( 0 >= ev.dwNumberOfBytesTransferred )
				{
					if ( 0 == clientSocket.disconnectOverlapped()
						 && ERROR_IO_PENDING != WSAGetLastError() )
					{
						// Exception
						std::cerr << "Disconnection from a client failed.\n";
						clientSocket.close( );
						clientS[ clientIdx ] = Socket( Socket::Type::TCP );
						if ( -1 == iocp.add(clientS[clientIdx], clientIdx) )
						{
							// Exception
							// Break twice
							goto cleanUp;
						}
					}
					else
					{
						clientSocket.pend( );
					}
					//TODO
					queue.remove( clientIdx );
					candidateS.emplace_back( clientIdx );
#ifdef _DEBUG
					std::cout << "A client left. (Now " << CAPACITY-candidateS.size() << '/' << CAPACITY << " connections.)\n";
#endif
				}
				else
				{
					switch ( clientSocket.completedWork( ) )
					{
						case Socket::CompletedWork::SEND:
							if ( true == clientSocket.hasTicket( ) )
							{
								if ( 0 == clientSocket.disconnectOverlapped( )
									 && ERROR_IO_PENDING != WSAGetLastError( ) )
								{
									// Exception
									std::cerr << "Disconnection from a client failed.\n";
									clientSocket.close( );
									clientS[ clientIdx ] = Socket( Socket::Type::TCP );
									if ( -1 == iocp.add(clientS[clientIdx], clientIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
								}
								else
								{
									clientSocket.pend( );
								}
								//TODO
								queue.remove( clientIdx );
								candidateS.emplace_back( clientIdx );
#ifdef _DEBUG
								std::cout << "A client left with a ticket. (Now " << CAPACITY - clientS.size() << " connections remain.)\n";
#endif
								clientSocket.earnTicket( false );
							}
							// When a queue number has been sended to a client,
							else
							{
								if ( 0 != clientSocket.receiveOverlapped( )
										&& ERROR_IO_PENDING != WSAGetLastError( ) )
								{
									// Exception
									std::cerr << "Receiving from a client failed.\n";
									clientSocket.close( );
									clientS[ clientIdx ] = Socket( Socket::Type::TCP );
									if ( -1 == iocp.add( clientS[clientIdx], clientIdx ) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
									//TODO
									queue.remove( clientIdx );
									candidateS.emplace_back( clientIdx );
									PrintLeavingE( CAPACITY - candidateS.size() );
								}
								else
								{
									clientSocket.pend( );
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

		if ( true == wasBoatful && 0 < candidateS.size( ) )
		{
			wasBoatful = false;
			const index nextCandidateClientIdx = candidateS.front( );
			if ( 0 == listener.acceptOverlapped( clientS[ nextCandidateClientIdx ] )
				 && ERROR_IO_PENDING != WSAGetLastError( ) )
			{
				// Exception
				std::cerr << "Overlapped acceptEx failed.\n";
				break;
			}
			listener.pend( );
		}
	}
cleanUp:
	//!IMPORTANT: Must release all the overlapped I/O resources in hand before closing the server.
	//	 		  Otherwise, overlapped I/O still runs on O/S background.
	toMainServer->close( );
	listener.close( );
	uint32_t pendingYet = 0u;
	for ( auto& it : clientS )
	{
		it.close( );
		if ( true == it.isPending() )
		{
			++pendingYet;
		}
	}
	std::cout << "Server gets closed.\n";
	while ( 0 < pendingYet || true == listener.isPending() || true == toMainServer->isPending() )
	{
		IOCPEvent event;
		iocp.wait( event, 100 );
		for ( uint32_t i = 0; i != event.eventCount; ++i )
		{
			const index evIdx = (index)event.events[ i ].lpCompletionKey;
			// When event comes from the main server,
			if ( MAIN_SERV_IDX == evIdx )
			{
				toMainServer->pend( false );
			}
			// When event comes from listener,
			else if ( LISTENER_IDX == evIdx	)
			{
				listener.pend( false );
			}
			// When event comes from TCP,
			else
			{
				clientS[ evIdx ].pend( false );
				--pendingYet;
			}
		}
	}
	clientS.clear( );
	
	WSACleanup( );
	std::cout << "Server has been closed successfully." << std::endl;

	return 0;
}