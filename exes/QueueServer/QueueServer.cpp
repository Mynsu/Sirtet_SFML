#include "pch.h"

//TODO: 소켓 핸들 랜덤이 아닌데?

//DOING: 로직

const uint32_t CAPACITY = 10u;
// Capacity in the main server defaults to 10000u.
// You can resize it indirectly here without re-compliing or rescaling the main server.
// !IMPORTANT: This must be less than the real capacity of the server.
const uint32_t MAIN_SERVER_CAPACITY = 1u;
constexpr char TAG_INVITATION[ ] = "inv:";
constexpr uint8_t TAG_INVITATION_LEN = ::util::hash::Measure( TAG_INVITATION );
const Dword VERSION = 8191015;
// Recommended to be renewed periodically for security.
const int ADDITIVE = 1246;
constexpr HashedKey ENCRYPTED_INVITATION = ::util::hash::Digest(VERSION+ADDITIVE);
constexpr char TAG_TICKET[ ] = "t:";
constexpr char TAG_ORDER_IN_QUEUE[ ] = "qL:";
const uint16_t LISTENER_PORT = 10000u;

volatile bool IsWorking = true;
void ProcessSignal( int signal )
{
	if ( SIGINT == signal )
	{
		IsWorking = false;
	}
}
inline void PrintLeavingE( const size_t numCandidates )
{
	std::cout << "A client left due to an error. (Now " << CAPACITY-numCandidates << '/' << CAPACITY << " connections.)\n";
}
inline int ConnectToMainServer( std::unique_ptr<Socket>& toMainServer )
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
// Try to reconnect to the main server.
inline int ResetMainServerSocket( std::unique_ptr<Socket>& toMainServer )
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

class Postman
{
public:
	inline Postman( )
		: mHasStuff( false )
	{ }
	inline ~Postman( ) = default;

	inline void pack( const char* TAG, const std::string& untaggedStuff, const char separator = ' ' )
	{
		mHasStuff = true;
		mBox << TAG << untaggedStuff << separator;
	}
	inline void pack( const std::string& taggedStuff, const char separator = ' ' )
	{
		mHasStuff = true;
		mBox << taggedStuff << separator;
	}
	inline bool hasStuff( ) const
	{
		return mHasStuff;
	}
	inline std::string package( ) const
	{
		return mBox.str( );
	}
private:
	bool mHasStuff;
	std::stringstream mBox;
};

enum Status
{
	VERIFYING_SALT,
	AWAITING_POPULATION,
};

int main( )
{
	signal( SIGINT, &ProcessSignal );
	
	WSAData w;
	WSAStartup( MAKEWORD(2, 2), &w );
	
	// Allocated in order to replace the socket immediately, differently from DisconnectEx(...).
	std::unique_ptr< Socket > socketToMainServer( std::make_unique<Socket>(Socket::Type::TCP) );
	if ( -1 == socketToMainServer->bind(EndPoint::Any) )
	{
		// Exception
		std::cerr << "Binding to-main-server socket failed.\n";
		socketToMainServer->close( );
		WSACleanup( );
		return -1;
	}
	if ( -1 == ConnectToMainServer(socketToMainServer) )
	{
		// Exception
		socketToMainServer->close( );
		WSACleanup( );
		return -1;
	}
	using index = uint32_t;
	const index MAIN_SERV_IDX = CAPACITY;
	IOCP iocp( 2 );
	std::cout << "\n##########\n### QUEUE SERVER\n##########\n\nWhat is today's salt, Sir?" << std::endl;
	std::string todaysSalt;
	std::cin >> todaysSalt;
	std::string encryptedSalt( std::to_string(::util::hash::Digest(todaysSalt.data(), 
																	static_cast<uint8_t>(todaysSalt.size()))) );
	// NOTE: Skipped because a socket isn't pending at first.
	///socketToMainServer.pend( false );
	int tolerance = 0;
	if ( -1 == iocp.add(*socketToMainServer, MAIN_SERV_IDX) )
	{
		// Exception
		std::cerr << "Adding to-main-server socket into IOCP failed.\n";
		socketToMainServer->close( );
		WSACleanup( );
		return -1;
	}
	// Acquiring how much room remains in the main server.
	while ( -1 == socketToMainServer->sendOverlapped(encryptedSalt.data(),encryptedSalt.size())
		 && ERROR_IO_PENDING != WSAGetLastError() )
	{
		// Exception
		std::cerr << "Sending to main server failed.\n";
		++tolerance;
		if ( 3 == tolerance || -1 == ResetMainServerSocket(socketToMainServer) )
		{
			socketToMainServer->close( );
			WSACleanup( );
			return -1;
		}
	}
	socketToMainServer->pend( );
	std::bitset< 8 > status;
	status.set( Status::VERIFYING_SALT, 1 );
	status.set( Status::AWAITING_POPULATION, 1 );

	Socket listener( Socket::Type::TCP_LISTENER );
	{
		char ipAddress[ ] = "192.168.219.102";
		if ( -1 == listener.bind(EndPoint(ipAddress, LISTENER_PORT)) )
		{
			// Exception
			std::cerr << "Binding listener failed.\n";
			socketToMainServer->close( );
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
		std::cerr << "Adding listener into ICOP failed.\n";
		socketToMainServer->close( );
		listener.close( );
		IOCPEvent event;
		iocp.wait( event, INFINITE );
		WSACleanup( );
		return -1;
	}
	std::vector< Socket > clientS;
	clientS.reserve( CAPACITY );
	container::IteratoredQueue< index > candidateS;
	for ( uint32_t i = 0u; i != CAPACITY; ++i )
	{
		clientS.emplace_back( Socket::Type::TCP );
		if ( -1 == iocp.add(clientS.at(i), i) )
		{
			// Exception
			std::cerr << "Adding a client into ICOP failed.\n";
			socketToMainServer->close( );
			listener.close( );
			IOCPEvent event;
			iocp.wait( event, INFINITE );
			WSACleanup( );
			clientS.clear( );
			break;
		}
		candidateS.emplace_back( i );
	}
	{
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
	}
	listener.pend( );
	std::cout << "Ready and verifying the today's salt.\n";

	bool wasBoatful = false;
	int roomInMainServer = 0;
	std::list< index > queue;
	IOCPEvent event;
	std::chrono::high_resolution_clock::time_point old0, old1;
	while ( true == IsWorking )
	{
		iocp.wait( event, 100 );
		Postman postmanToMainServer;
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
				Socket& candidateSocket = clientS[ candidateClientIdx ];
				if ( -1 == candidateSocket.updateAcceptContext(listener) )
				{
					// Exception
					// NOTE: Break and break
					goto cleanUp;
				}
				// When acception is successful,
				candidateS.pop_front( );
				Socket& clientSocket = candidateSocket;
				const index clientIdx = candidateClientIdx;
#ifdef _DEBUG
				std::cout << "A new client " << clientIdx << " wants to join the main server. (Now " << CAPACITY-candidateS.size() << "/" << CAPACITY << " connections.)\n";
#endif
				if ( -1 == clientSocket.receiveOverlapped()
					 && ERROR_IO_PENDING != WSAGetLastError() )
				{
					// Exception
					std::cerr << "Pending reception of a new client failed.\n";
					clientSocket.close( );
					clientS[ clientIdx ] = Socket( Socket::Type::TCP, Socket::CompletedWork::DISCONNECT );
					if ( -1 == iocp.add(clientS[clientIdx], clientIdx) )
					{
						// Exception
						// Break twice.
						goto cleanUp;
					}
					candidateS.emplace_back( clientIdx );
					PrintLeavingE( candidateS.size() );
				}
				else
				{
					clientSocket.pend( );
				}

				// Reloading the next candidate.
				// When room for the next client in THIS queue server, not the main server, remains yet,
				if ( 0 < candidateS.size() )
				{
					const index nextCandidateClientIdx = candidateS.front( );
					if ( 0 == listener.acceptOverlapped(clientS[nextCandidateClientIdx])
						 && ERROR_IO_PENDING != WSAGetLastError() )
					{
						// Exception
						std::cerr << "Overlapped acceptEx failed.\n";
						goto cleanUp;
					}
					listener.pend( );
				}
				else
				{
#ifdef _DEBUG
					std::cout << "Boatful!\n";
#endif
					wasBoatful = true;
				}
			}
			////
			// When event comes from the main server,
			////
			else if ( MAIN_SERV_IDX == (index)ev.lpCompletionKey )
			{
				socketToMainServer->pend( false );
				if ( 0 >= ev.dwNumberOfBytesTransferred )
				{
					if ( 1 == status[Status::VERIFYING_SALT] )
					{
						std::cerr << "Today's salt doesn't match that of the main server.\n";
						IsWorking = false;
					}
					else
					{
						std::cerr << "Main server closed or died.\n";
						if ( -1 == ResetMainServerSocket(socketToMainServer) )
					{
						// Exception
						// NOTE: Brake twice.
						goto cleanUp;
					}
					}
				}
				else
				{
					switch( socketToMainServer->completedWork() )
					{
						case Socket::CompletedWork::RECEIVE:
							if ( true == status[Status::VERIFYING_SALT] )
							{
								status.set(Status::VERIFYING_SALT, false );
							}
							// When having received population in the main server,
							if ( 1 == status[Status::AWAITING_POPULATION] )
							{
								// NOTE: Assuming that there's a message about nothing but population in and from the main server.
								status.set( Status::AWAITING_POPULATION, 0 );
								// NOTE: Assuming that no tag is attached.
								const int pop = std::atoi( socketToMainServer->receivingBuffer() );
								// When population is out of range,
								if ( pop<0 || MAIN_SERVER_CAPACITY<pop )
								{
									// Asking again the main server how many clients are there.
									postmanToMainServer.pack( encryptedSalt );
									status.set( Status::AWAITING_POPULATION, 1 );
									std::cout << "Asking again the main server how many clients are there.\n";
									break;
								}
								// Otherwise, when population is in range,
								roomInMainServer = static_cast< int >( MAIN_SERVER_CAPACITY ) - pop;
#ifdef _DEBUG
								std::cout << "Room in the main server: " << roomInMainServer << std::endl;
#endif
							}
							break;
						case Socket::CompletedWork::SEND:
							// When having asked population in the main server or having sent a ticket,
							// Pending reception later, not here now.
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
			////
			// When event comes from a client
			////
			else
			{
				const index clientIdx = (index)ev.lpCompletionKey;
				Socket& clientSocket = clientS[ clientIdx ];
				clientSocket.pend( false );
				if ( Socket::CompletedWork::DISCONNECT == clientSocket.completedWork( ) )
				{
					const int res = candidateS.contains( clientIdx );
					if ( -1 == res )
					{
						std::cerr << "Duplicate candidates found.\n";
					}
					else if ( 0 == res )
					{
						candidateS.emplace_back( clientIdx );
#ifdef _DEBUG
						if ( true == clientSocket.hasTicket() )
						{
							std::cout << "A client " << clientIdx << "  left with a ticket. (Now " << CAPACITY-clientS.size( ) << '/' << CAPACITY << " connections.)\n";
						}
						else
#endif
						{
							std::cout << "A client " << clientIdx << " left. (Now " << CAPACITY-candidateS.size() << '/' << CAPACITY << " connections.)\n";
						}
						clientSocket.earnTicket( false );
					}
				}
				// 0 in disconnection, <0 in error.
				else if ( 0 >= ev.dwNumberOfBytesTransferred )
				{
					if ( 0 == clientSocket.disconnectOverlapped()
						 && ERROR_IO_PENDING != WSAGetLastError() )
					{
						// Exception
						std::cerr << "Forced to disconnect a client failed.\n";
						clientSocket.close( );
						clientS[ clientIdx ] = Socket( Socket::Type::TCP );
						if ( -1 == iocp.add(clientS[clientIdx], clientIdx) )
						{
							// Exception
							// Break twice
							goto cleanUp;
						}
						candidateS.emplace_back( clientIdx );
						PrintLeavingE( candidateS.size() );
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
						case Socket::CompletedWork::SEND:
							// When a ticket has been
							if ( true == clientSocket.hasTicket( ) )
							{
								if ( 0 == clientSocket.disconnectOverlapped( )
									 && ERROR_IO_PENDING != WSAGetLastError( ) )
								{
									// Exception
									std::cerr << "Forced to disconnect a client.\n";
									clientSocket.close( );
									clientS[ clientIdx ] = Socket( Socket::Type::TCP, Socket::CompletedWork::DISCONNECT );
									if ( -1 == iocp.add(clientS[clientIdx], clientIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
									clientSocket.earnTicket( false );
									candidateS.emplace_back( clientIdx );
									std::cout << "A client left with a ticket. (Now " << CAPACITY-clientS.size( ) << '/' << CAPACITY << " connections.)\n";
								}
								else
								{
									clientSocket.pend( );
								}
							}
							// When a queue number has been sended to a client in the queue line,
							///else
							///{
								// Doing nothing.
							///}
							break;
						case Socket::CompletedWork::RECEIVE:
							// When having received the genuine invitation,
							// NOTE: Assuming that the message only comes from a new connection.
							// NOTE: Assuming that there're no impurities but an invitation in the message.
							if ( char* const rcvBuf = clientSocket.receivingBuffer( );
								ENCRYPTED_INVITATION == static_cast<HashedKey>(std::atoll(&rcvBuf[TAG_INVITATION_LEN])) )
							{
								queue.emplace_back( clientIdx );
#ifdef _DEBUG
								std::cout << "Client " << clientIdx <<
									" with the genuine invitation has been added into the queue line.\n";
#endif
							}
							// Otherwise, when a client connected without any genuine invitation,
							else
							{
								std::cout << "A client connected without any genuine invitation.\n";
								if ( 0 == clientSocket.disconnectOverlapped()
									 && ERROR_IO_PENDING != WSAGetLastError() )
								{
									// Exception
									clientSocket.close( );
									clientS[ clientIdx ] = Socket( Socket::Type::TCP, Socket::CompletedWork::DISCONNECT );
									if ( -1 == iocp.add(clientS[clientIdx], clientIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
									candidateS.emplace_back( clientIdx );
									PrintLeavingE( candidateS.size() );
								}
								else
								{
									clientSocket.pend( );
								}
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

		// When room in the main server remains yet,
		for ( auto it = queue.cbegin(); queue.cend()!=it && 0<roomInMainServer; )
		{
			Socket& clientSocket = clientS[ *it ];
			const std::string undigestedID( std::to_string(clientSocket.handle()) + encryptedSalt );
			std::string ticket( TAG_TICKET + std::to_string(::util::hash::Digest(undigestedID.data(),
																					static_cast<uint8_t>(undigestedID.size()))) );
#ifdef _DEBUG
			if ( false != clientSocket.isPending() ) __debugbreak( );
#endif
			clientSocket.pend( false );
			// First to a client, last to the main server.
			// Otherwise, it's burdensome to get rid of a copy of the ticket from the main server,
			// actually not issued to the client.
			//
			// Issuing the ticket to the client.
			if ( -1 == clientSocket.sendOverlapped(ticket.data(),ticket.size())
					&& ERROR_IO_PENDING != WSAGetLastError() )
			{
				// Exception
				std::cerr << "Sending a ticket to a client waiting in the queue line failed.\n";
				clientSocket.close( );
				clientS[ *it ] = Socket( Socket::Type::TCP, Socket::CompletedWork::DISCONNECT );
				if ( -1 == iocp.add( clientS[ *it ], *it ) )
				{
					// Exception
					// Break twice
					goto cleanUp;
				}
				candidateS.emplace_back( *it );
				PrintLeavingE( candidateS.size( ) );
			}
			else
			{
#ifdef _DEBUG
				std::cout << "Sending a ticket " << ticket.data( ) << " to a client " << *it << " succeeded.\n";
#endif
				clientSocket.pend( );
				clientSocket.earnTicket( );
				// Trying to send the issued ticket to the main server.
				postmanToMainServer.pack( ticket );
				--roomInMainServer;
#ifdef _DEBUG
				std::cout << "Sending a ticket " << ticket.data( ) << " to the main server, too.\n";
#endif
			}
			it = queue.erase( it );
		}

		const auto now = std::chrono::high_resolution_clock::now( );
		// Notifying their own order in the queue line.
		for ( auto it = queue.cbegin(); queue.cend()!=it && std::chrono::seconds(1)<now-old0; )
		{
			// Reset
			old0 = now;
			Socket& clientSocket = clientS[ *it ];
			std::string itsOrderInQueueLine( TAG_ORDER_IN_QUEUE + std::to_string(*it) );
#ifdef _DEBUG
			if ( false != clientSocket.isPending( ) ) __debugbreak( );
#endif
			clientSocket.pend( false );
			if ( -1 == clientSocket.sendOverlapped(itsOrderInQueueLine.data(),itsOrderInQueueLine.size())
					&& ERROR_IO_PENDING != WSAGetLastError( ) )
			{
				// Exception
				std::cerr << "Sending its order to the client waiting in the queue line failed.\n";
				clientSocket.close( );
				clientS[ *it ] = Socket( Socket::Type::TCP, Socket::CompletedWork::DISCONNECT );
				if ( -1 == iocp.add( clientS[ *it ], *it ) )
				{
					// Exception
					// Break twice
					goto cleanUp;
				}
				candidateS.emplace_back( *it );
				it = queue.erase( it );
				PrintLeavingE( candidateS.size( ) );
			}
			else
			{
#ifdef _DEBUG
				std::cout << "Sending its order to the client " << *it << " waiting in the queue line succeeded.\n";
#endif
				clientSocket.pend( );
				++it;
			}
		}

		// When there's no more room in the main server,
		if ( 0 == roomInMainServer && 0 == status[Status::AWAITING_POPULATION]
			 && std::chrono::seconds(1)<now-old1 )
		{
			// Reset
			old1 = now;
			// Asking the main server how many clients are there.
			postmanToMainServer.pack( encryptedSalt );
			status.set( Status::AWAITING_POPULATION, 1 );
#ifdef _DEBUG
			std::cout << "Asking the main server how many clients are there.\n";
#endif
		}

		// When there's something to send to the main server,
		if ( true == postmanToMainServer.hasStuff() )
		{
#ifdef _DEBUG
			if ( false != socketToMainServer->isPending( ) ) __debugbreak( );
#endif
			socketToMainServer->pend( false );
			std::string package( postmanToMainServer.package() );
			tolerance = 0;
			while ( -1 == socketToMainServer->sendOverlapped(package.data(),package.size())
					&& ERROR_IO_PENDING != WSAGetLastError( ) )
			{
				// Exception
				std::cerr << "Sending the issued ticket to main server failed.\n";
				++tolerance;
				if ( 3 == tolerance || -1 == ResetMainServerSocket( socketToMainServer )
					 || -1 == iocp.add( *socketToMainServer, MAIN_SERV_IDX ) )
				{
					// Exception
					// Break twice.
					goto cleanUp;
				}
			}
			socketToMainServer->pend( );
		}
		// When there's nothing to send to the main server,
		else
		{
			// When having asked population in the previous while loop, not this loop,
			if ( 1 == status[Status::AWAITING_POPULATION] )
			{
#ifdef _DEBUG
				if ( false != socketToMainServer->isPending( ) ) __debugbreak( );
				std::cout << "Waiting for the main server's answer how many clients are there.\n";
#endif
				tolerance = 0;
				while ( -1 == socketToMainServer->receiveOverlapped( )
						&& ERROR_IO_PENDING != WSAGetLastError( ) )
				{
					// Exception
					std::cerr << "Receiving from the main server failed.\n";
					++tolerance;
					if ( 3 == tolerance || -1 == ResetMainServerSocket( socketToMainServer )
						 || -1 == iocp.add( *socketToMainServer, MAIN_SERV_IDX ) )
					{
						// Exception
						// Break twice.
						goto cleanUp;
					}
				}
				socketToMainServer->pend( );
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
	socketToMainServer->close( );
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
	while ( 0 < pendingYet || true == listener.isPending() || true == socketToMainServer->isPending() )
	{
		IOCPEvent event;
		iocp.wait( event, 100 );
		for ( uint32_t i = 0; i != event.eventCount; ++i )
		{
			const index evIdx = (index)event.events[ i ].lpCompletionKey;
			// When event comes from the main server,
			if ( MAIN_SERV_IDX == evIdx )
			{
				socketToMainServer->pend( false );
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