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
	std::cout << "A client left due to an error. (Now " << population << "/" << CAPACITY << " connections.)\n";
}

class Client
{
public:
	Client( ) = delete;
	Client( const Socket::Type type )
		: mSocket( type )
	{ }
	Socket& socket( )
	{
		return mSocket;
	}
	void setSocket( const Socket& socket )
	{
		mSocket = socket;
	}
private:
	Socket mSocket;
};

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
		const uint16_t port = 54321u;
		if ( -1 == listener.bind(EndPoint(ipAddress, port)) )
		{
			// Exception
			listener.close( );
			WSACleanup( );
			std::cerr << "Listener binding failed.\n";
			return -1;
		}
	}
	listener.listen( );
	using index = uint32_t;
	const index LISTENER_IDX = CAPACITY;
	if ( -1 == iocp.add(listener, LISTENER_IDX) )
	{
		// Exception
		listener.close( );
		WSACleanup( );
		std::cerr << "Adding listener into ICOP failed.\n";
		return -1;
	}
	std::vector< Client > clientS;
	clientS.reserve( CAPACITY );
	// Must use push_back(or emplace_back), front and pop_front only, like std::queue.
	std::list< index > candidateS;
	for ( uint32_t i = 0; i != CAPACITY; ++i )
	{
		clientS.emplace_back( Socket::Type::TCP );
		if ( -1 == iocp.add(clientS.at(i).socket(), i) )
		{
			listener.close( );
			WSACleanup( );
			clientS.clear( );
			return -1;
		}
		candidateS.emplace_back( i );
	}
	const int result = listener.acceptOverlapped( clientS.at(candidateS.front()).socket() );
	if ( 1 != result && ERROR_IO_PENDING != WSAGetLastError( ) )
	{
		// Exception
		listener.close( );
		clientS.clear( );
		WSACleanup( );
		if ( 0 == result )
		{
			std::cerr << "Overlapped acceptEx failed.\n";
		}
		else
		{
			std::cerr << "Getting AcceptEx failed.\n";
		}
		return -1;
	}
	listener.pend( );

	std::cout << "##########\n### MAIN SERVER\n##########\n\nWhat is today's salt, Sir?" << std::endl;
	char todaysSalt[ 10 ];
	std::cin >> todaysSalt;
	std::cout << "Ready.\n";
	const uint8_t saltSize = static_cast<uint8_t>(std::strlen(todaysSalt));
	const HashedKey refinedSalt = ::util::hash::Digest( todaysSalt, saltSize );
	index queueServerIdx = -1;
	std::unordered_set< HashedKey > ticketS;
	bool wasBoatful = false;
	IOCPEvent event;
	while ( true == IsWorking )
	{
		iocp.wait( event, 100 );
		for ( uint32_t i = 0u; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[ i ];
			////
			// When event comes from listener,
			////
			if ( LISTENER_IDX == (index)ev.lpCompletionKey && 0 < candidateS.size() )
			{
				listener.pend( false );
				const index candidateClientIdx = candidateS.front( );
				Socket& candidateClientSocket = clientS[ candidateClientIdx ].socket( );
				if ( -1 == candidateClientSocket.updateAcceptContext(listener) )
				{
					// Exception
					// NOTE: Break and break
					goto cleanUp;
				}
				
				// When acception is successful,
				Socket& clientSocket = candidateClientSocket;
				const index clientIdx = candidateClientIdx;
				if ( -1 == clientSocket.receiveOverlapped( )
					&& ERROR_IO_PENDING != WSAGetLastError( ) )
				{
					// Exception
					std::cerr << "Receiving from a client failed.\n";
					clientSocket.close( );
					clientS[ clientIdx ].setSocket( Socket(Socket::Type::TCP) );
					if ( -1 == iocp.add( clientS[clientIdx].socket(), clientIdx) )
					{
						// Exception
						// Break twice.
						goto cleanUp;
					}
					PrintLeavingE( CAPACITY - candidateS.size( ) );
				}
				else
				{
					candidateS.pop_front( );
					clientSocket.pend( );
#ifdef _DEBUG
				std::cout << "A new client joined. (Now " << CAPACITY-candidateS.size() << "/" << CAPACITY << " connections.)\n";
#endif
				}

				// Reloading the next candidate.
				if ( 0 < candidateS.size() )
				{
					const index nextCandidateIdx = candidateS.front( );
					if ( 0 == listener.acceptOverlapped(clientS[nextCandidateIdx].socket())
						 && ERROR_IO_PENDING != WSAGetLastError() )
					{
						// Exception
						std::cerr << "Overlapped acceptEx failed.\n";
						goto cleanUp;
					}
					else
					{
						listener.pend( );
					}
				}
				else
				{
					wasBoatful = true;
				}
			}
			////
			// When event comes from the queue server,
			////
			else if ( queueServerIdx == (index)ev.lpCompletionKey )
			{
				Socket& queueServerSocket = clientS[ queueServerIdx ].socket( );
				queueServerSocket.pend( false );
				if ( Socket::CompletedWork::DISCONNECT == queueServerSocket.completedWork( ) )
				{
					candidateS.emplace_back( queueServerIdx );
					queueServerIdx = -1;
				}
				// 0 in disconnection, <0 in error.
				else if ( 0 >= ev.dwNumberOfBytesTransferred )
				{
					if ( 0 == queueServerSocket.disconnectOverlapped( )
						 && ERROR_IO_PENDING != WSAGetLastError( ) )
					{
						// Exception
						std::cerr << "Disconnection from the queue server failed.\n";
						queueServerSocket.close( );
						clientS[ queueServerIdx ].setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
						if ( -1 == iocp.add(clientS[queueServerIdx].socket(), queueServerIdx) )
						{
							// Exception
							// Break twice
							goto cleanUp;
						}
					}
					else
					{
						queueServerSocket.pend( );
					}
#ifdef _DEBUG
					std::cerr << "The queue server disconnected.\n";
#endif
				}
				else
				{
					switch ( queueServerSocket.completedWork( ) )
					{
						case Socket::CompletedWork::RECEIVE:
						{
							char* rcvBuf = queueServerSocket.receivingBuffer( );
							// When the queue server asked how much room remains,
							if ( refinedSalt == ::util::hash::Digest(rcvBuf,saltSize) )
							{
								const uint32_t room = CAPACITY - static_cast< uint32_t >( candidateS.size() );
								std::string population( std::to_string(room) );
								if ( -1 == queueServerSocket.sendOverlapped(population.data(), (ULONG)population.size())
										&& ERROR_IO_PENDING != WSAGetLastError( ) )
								{
									// Exception
									std::cerr << "Sending population to the queue server failed.\n";
									queueServerSocket.close( );
									clientS[ queueServerIdx ].setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
									if ( -1 == iocp.add(clientS[queueServerIdx].socket(), queueServerIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
									//TODO
									//candidateS.emplace_back( queueServerIdx );
								}
								else
								{
									queueServerSocket.pend( );
#ifdef _DEBUG
									std::cout << "Population has been requested and sent: " << population.data( ) << std::endl;
#endif
								}
							}
							else
							{
#ifdef _DEBUG
								std::cout << "A copy of a ticket arrived: " << rcvBuf << std::endl;
#endif
								ticketS.emplace( std::atoi(rcvBuf) );
								if ( -1 == queueServerSocket.receiveOverlapped()
									 && ERROR_IO_PENDING != WSAGetLastError() )
								{
									// Exception
									std::cerr << "Receiving from the queue server failed.  Disconnected.\n";
									queueServerSocket.close( );
									clientS[ queueServerIdx ].setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
									if ( -1 == iocp.add(clientS[queueServerIdx].socket(), queueServerIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
									//TODO
									//candidateS.emplace_back( queueServerIdx );
								}
								else
								{
									queueServerSocket.pend( );
								}
							}
							break;
						}
						case Socket::CompletedWork::SEND:
						{
							if ( -1 == queueServerSocket.receiveOverlapped()
								 && ERROR_IO_PENDING != WSAGetLastError() ) //10053, 10054
							{
								// Exception
								std::cerr << "Receiving from the queue server failed.  Disconnected.\n";
								queueServerSocket.close( );
								clientS[ queueServerIdx ].setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
								if ( -1 == iocp.add(clientS[queueServerIdx].socket(), queueServerIdx) )
								{
									// Exception
									// Break twice
									goto cleanUp;
								}
								//TODO
								candidateS.emplace_back( queueServerIdx );
							}
							else
							{
								queueServerSocket.pend( );
							}
							break;
						}
						default:
#ifdef _DEBUG
							__debugbreak( );
#else
							__assume(0);
#endif
					}
				}
			}
			////
			// When event comes from clients,
			////
			else
			{
				const index clientIdx = (index)ev.lpCompletionKey;
				Socket& clientSocket = clientS[ clientIdx ].socket( );
				clientSocket.pend( false );
				if ( Socket::CompletedWork::DISCONNECT == clientSocket.completedWork( ) )
				{
					candidateS.emplace_back( clientIdx );
#ifdef _DEBUG
					std::cout << "A client left. (Now " << CAPACITY-candidateS.size( ) << "/" << CAPACITY << " connections.)\n";
#endif
				}
				// 0 in disconnection, <0 in error.
				else if ( 0 >= ev.dwNumberOfBytesTransferred )
				{
					if ( 0 == clientSocket.disconnectOverlapped( )
						 && ERROR_IO_PENDING != WSAGetLastError( ) )
					{
						// Exception
						std::cerr << "Disconnection from a client failed.\n";
						clientSocket.close( );
						clientS[ clientIdx ].setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
						if ( -1 == iocp.add(clientS[clientIdx].socket(), clientIdx) )
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
				}
				else
				{
					switch ( clientSocket.completedWork( ) )
					{
						case Socket::CompletedWork::RECEIVE:
						{
							char* rcvBuf = clientSocket.receivingBuffer( );
							// When event comes from the queue server
							if ( refinedSalt == ::util::hash::Digest(rcvBuf, saltSize) )
							{
#ifdef _DEBUG
								std::cout << "Population has been requested.\n";
#endif
								queueServerIdx = clientIdx;
								std::string population( std::to_string(CAPACITY-candidateS.size()) );
								Socket& queueServerSocket = clientSocket;
#ifdef _DEBUG
								if ( false != queueServerSocket.isPending() ) __debugbreak( );
#endif
								queueServerSocket.pend( false );
								if ( -1 == queueServerSocket.sendOverlapped(population.data(), (ULONG)population.size())
									 && ERROR_IO_PENDING != WSAGetLastError() )
								{
									// Exception
									std::cerr << "Sending population to the queue server failed.\n";
									queueServerSocket.close( );
									clientS[ clientIdx ].setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
									if ( -1 == iocp.add(clientS[clientIdx].socket(), clientIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
								}
								else
								{
									queueServerSocket.pend( );
#ifdef _DEBUG
									std::cout << "Population has been sent: " << population.data( ) << std::endl;
#endif
								}
							}
							// When event comes from a new client,
							else if ( false == clientSocket.hasTicket() )
							{
								// When there's no one matching the ticket from this client,
								if ( ticketS.cend() == ticketS.find(std::atoi(rcvBuf)) )
								{
									std::cout << "Someone tried to connect with no valid ticket.\n";
									if ( 0 == clientSocket.disconnectOverlapped( )
										 && ERROR_IO_PENDING != WSAGetLastError( ) )
									{
										// Exception
										clientSocket.close( );
										clientS[ clientIdx ].setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
										if ( -1 == iocp.add(clientS[clientIdx].socket(), clientIdx ) )
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
									PrintLeavingE( CAPACITY - candidateS.size( ) - 1u );
								}
								else
								{
									//TODO
									if ( -1 == clientSocket.receiveOverlapped( )
										 && ERROR_IO_PENDING != WSAGetLastError( ) )
									{
										// Exception
										std::cerr << "Receiving from a client failed.\n";
										// Exception
										std::cerr << "Disconnection from a client failed.\n";
										clientSocket.close( );
										clientS[ clientIdx ].setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
										if ( -1 == iocp.add( clientS[ clientIdx ].socket( ), clientIdx ) )
										{
											// Exception
											// Break twice
											goto cleanUp;
										}
										PrintLeavingE( CAPACITY - candidateS.size( ) - 1u );
									}
									else
									{
										clientSocket.pend( );
									}
								}
							}
							else
							{
								//TODO
							}
							break;
						}
						case Socket::CompletedWork::SEND:
							if ( -1 == clientSocket.receiveOverlapped()
								 && ERROR_IO_PENDING != WSAGetLastError( ) )
							{
								// Exception
								std::cerr << "Receiving from a client failed.\n";
								clientSocket.close( );
								clientS[ clientIdx ].setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
								if ( -1 == iocp.add(clientS[clientIdx].socket(), clientIdx) )
								{
									// Exception
									// Break twice
									goto cleanUp;
								}
								PrintLeavingE( CAPACITY - candidateS.size( ) - 1u );
							}
							else
							{
								clientSocket.pend( );
							}
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
			if ( 0 == listener.acceptOverlapped( clientS[nextCandidateClientIdx].socket() )
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
	listener.close( );
	uint32_t pendingYet = 0u;
	for ( auto& it : clientS )
	{
		Socket& socket = it.socket( );
		socket.close( );
		if ( true == socket.isPending( ) )
		{
			++pendingYet;
		}
	}
	std::cout << "Server gets closed.\n";
	while ( 0 < pendingYet || true == listener.isPending( ) )
	{
		IOCPEvent event;
		iocp.wait( event, 100 );
		for ( uint32_t i = 0; i != event.eventCount; ++i )
		{
			const index evIdx = (index)event.events[ i ].lpCompletionKey;
			// When event comes from listener,
			if ( LISTENER_IDX == evIdx )
			{
				listener.pend( false );
			}
			// When event comes from TCP,
			else
			{
				clientS[ evIdx ].socket( ).pend( false );
				--pendingYet;
			}
		}
	}
	clientS.clear( );
	std::cout << "Server has been closed successfully." << std::endl;

	WSACleanup( );

	return 0;
}