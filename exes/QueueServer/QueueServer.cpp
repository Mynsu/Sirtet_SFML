#include "pch.h"

//TODO: 티켓 랜덤하게, RDBMS로 ID와 Password 매칭.

const uint32_t CAPACITY = 10000u;
// Capacity in the main server defaults to 10000u.
// You can resize it indirectly here without re-compliing or rescaling the main server.
// !IMPORTANT: This must be less than the real capacity of the server.
const uint32_t MAIN_SERVER_CAPACITY = 3u;
const Dword VERSION = 8191015;
constexpr HashedKey ENCRYPTED_INVITATION = ::util::hash::Digest(VERSION+ADDITIVE);
const uint16_t LISTENER_PORT = QUEUE_SERVER_PORT;

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
		std::cerr << "Failed to connect to the main server.\n";
		++tolerance;
		if ( 3 == tolerance )
		{
			return -1;
		}
		std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
	}
	std::cout << "Succeeded in connecting to the main server.\n";

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
		std::cerr << "Failed to bind to-main-server socket.\n";
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
		std::cerr << "Failed to bind to-main-server socket.\n";
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
	using Index = uint32_t;
	const Index MAIN_SERV_IDX = CAPACITY;
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
		std::cerr << "Failed to add to-main-server socket into IOCP.\n";
		socketToMainServer->close( );
		WSACleanup( );
		return -1;
	}
	// Acquiring how much room remains in the main server.
	while ( -1 == socketToMainServer->sendOverlapped(encryptedSalt.data(),encryptedSalt.size())
		 && ERROR_IO_PENDING != WSAGetLastError() )
	{
		// Exception
		std::cerr << "Failed to send Salt to main server.\n";
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
			std::cerr << "Failed to bind listener.\n";
			socketToMainServer->close( );
			IOCPEvent event;
			iocp.wait( event, INFINITE );
			WSACleanup( );
			return -1;
		}
	}
	listener.listen( );
	const Index LISTENER_IDX = CAPACITY + 1u;
	if ( -1 == iocp.add(listener, LISTENER_IDX) )
	{
		// Exception
		std::cerr << "Failed to add listener into ICOP.\n";
		socketToMainServer->close( );
		listener.close( );
		IOCPEvent event;
		iocp.wait( event, INFINITE );
		WSACleanup( );
		return -1;
	}
	std::vector< Socket > clientS;
	clientS.reserve( CAPACITY );
	container::IteratoredQueue< Index > candidateS;
	for ( uint32_t i = 0u; i != CAPACITY; ++i )
	{
		clientS.emplace_back( Socket::Type::TCP );
		if ( -1 == iocp.add(clientS.at(i), i) )
		{
			// Exception
			std::cerr << "Failed to add a client into ICOP.\n";
			socketToMainServer->close( );
			listener.close( );
			IOCPEvent event;
			iocp.wait( event, INFINITE );
			WSACleanup( );
			clientS.clear( );
			return -1;
		}
		candidateS.emplace_back( i );
	}
	auto forceDisconnection = [ &clientS, &iocp, &candidateS ]( const Index clientIndex ) -> int
	{
		Socket& clientSocket = clientS[ clientIndex ];
		clientSocket.close( );
		clientSocket = Socket( Socket::Type::TCP, Socket::CompletedWork::DISCONNECT );
		if ( -1 == iocp.add(clientSocket, clientIndex) )
		{
			// Exception
			return -1;
		}
		candidateS.emplace_back( clientIndex );
		return 0;
	};
	{
		const int result = listener.acceptOverlapped( clientS.at(candidateS.front()) );
		if ( 0 == result && ERROR_IO_PENDING != WSAGetLastError( ) )
		{
			// Exception
			std::cerr << "Failed to pend acception.\n";
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
	std::list< Index > queue;
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
			if ( LISTENER_IDX == (Index)ev.lpCompletionKey && 0 < candidateS.size() )
			{
				listener.pend( false );
				const Index candidateClientIdx = candidateS.front( );
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
				const Index clientIdx = candidateClientIdx;
#ifdef _DEBUG
				std::cout << "A new client " << clientIdx << " wants to join the main server. (Now " << CAPACITY-candidateS.size() << "/" << CAPACITY << " connections.)\n";
#endif
				if ( -1 == clientSocket.receiveOverlapped()
					 && ERROR_IO_PENDING != WSAGetLastError() )
				{
					// Exception
					std::cerr << "Failed to pend reception of a new client.\n";
					PrintLeavingE( candidateS.size() );
					if ( -1 == forceDisconnection(clientIdx) )
					{
						// Break twice.
						goto cleanUp;
					}
				}
				else
				{
					clientSocket.pend( );
				}

				// Reloading the next candidate.
				// When room for the next client in THIS queue server, not the main server, remains yet,
				if ( 0 < candidateS.size() )
				{
					const Index nextCandidateClientIdx = candidateS.front( );
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
			else if ( MAIN_SERV_IDX == (Index)ev.lpCompletionKey )
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
					const Socket::CompletedWork cmpl = socketToMainServer->completedWork( );
					if ( Socket::CompletedWork::RECEIVE == cmpl )
					{
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
								continue;
							}
							// Otherwise, when population is in range,
							roomInMainServer = static_cast< int >( MAIN_SERVER_CAPACITY ) - pop;
#ifdef _DEBUG
							std::cout << "Room in the main server: " << roomInMainServer << std::endl;
#endif
						}
					}
					else if ( Socket::CompletedWork::SEND == cmpl )
					{
						// When having asked population in the main server or having sent a ticket,
						// Pending reception later, not here now.
					}
					else
					{
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
				const Index clientIdx = (Index)ev.lpCompletionKey;
				Socket& clientSocket = clientS[ clientIdx ];
				clientSocket.pend( false );
				const Socket::CompletedWork cmpl = clientSocket.completedWork( );
				if ( Socket::CompletedWork::DISCONNECT == cmpl )
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
							std::cout << "A client " << clientIdx << " left with a ticket. (Now " << CAPACITY-clientS.size( ) << '/' << CAPACITY << " connections.)\n";
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
						std::cerr << "Failed to disconnect Client " << clientIdx << ".\n";
						PrintLeavingE( candidateS.size() );
						if ( -1 == forceDisconnection(clientIdx) )
						{
							// Exception
							// Break twice
							goto cleanUp;
						}
						continue;
					}
					clientSocket.pend( );
				}
				else
				{
					if ( Socket::CompletedWork::SEND == cmpl )
					{
						if ( true == clientSocket.hasTicket() )
						{
							if ( 0 == clientSocket.disconnectOverlapped( )
									&& ERROR_IO_PENDING != WSAGetLastError( ) )
							{
								// Exception
								std::cerr << "Failed to disconnect Client " << clientIdx << ".\n";
								if ( -1 == forceDisconnection(clientIdx) )
								{
									// Exception
									// Break twice
									goto cleanUp;
								}
								clientSocket.earnTicket( false );
								std::cout << "A client left with a ticket. (Now " << CAPACITY-clientS.size( ) << '/' << CAPACITY << " connections.)\n";
								continue;
							}
							clientSocket.pend( );
						}
						// When a queue number has been sended to a client in the queue line,
						///else
						///{
						// Done later.
						///}
					}
					else if ( Socket::CompletedWork::RECEIVE == cmpl )
					{
						// When having received the genuine invitation,
						// NOTE: Assuming that the message only comes from a new connection.
						// NOTE: Assuming that there're no impurities but an invitation in the message.
						if ( char* const rcvBuf = clientSocket.receivingBuffer( );
							///ENCRYPTED_INVITATION == static_cast<HashedKey>(std::atoll(&rcvBuf[TAG_INVITATION_LEN])) )
							ENCRYPTED_INVITATION == static_cast<HashedKey>(std::atoll(rcvBuf)) )
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
								std::cerr << "Failed to disconnect Client " << clientIdx << ".\n";
								PrintLeavingE( candidateS.size() );
								if ( -1 == forceDisconnection(clientIdx) )
								{
									// Exception
									// Break twice
									goto cleanUp;
								}
								continue;
							}
							clientSocket.pend( );
						}
					}
					else
					{
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
				std::cerr << "Failed to send a ticket to Client " << *it
					<< " waiting in the queue line.\n";
				if ( -1 == forceDisconnection(*it) )
				PrintLeavingE( candidateS.size( ) );
				{
					// Exception
					// Break twice
					goto cleanUp;
				}
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
				std::cout << "Sending a ticket " << ticket.data( ) << " to the main server succeeded, too.\n";
#endif
			}
			it = queue.erase( it );
		}

		const auto now = std::chrono::high_resolution_clock::now( );
		// Notifying their own order in the queue line.
		uint32_t order = 0u;
		for ( auto it = queue.cbegin(); queue.cend()!=it && std::chrono::seconds(1)<now-old0; )
		{
			// Reset
			old0 = now;

			Socket& clientSocket = clientS[ *it ];
			++order;
			std::string itsOrderInQueueLine( TAG_ORDER_IN_QUEUE + std::to_string(order) );
#ifdef _DEBUG
			if ( false != clientSocket.isPending( ) ) __debugbreak( );
#endif
			clientSocket.pend( false );
			if ( -1 == clientSocket.sendOverlapped(itsOrderInQueueLine.data(),itsOrderInQueueLine.size())
					&& ERROR_IO_PENDING != WSAGetLastError( ) )
			{
				// Exception
				std::cerr << "Failed to send its order to Client " << *it
					<< " waiting in the queue line.\n";
				PrintLeavingE( candidateS.size( ) );
				if ( -1 == forceDisconnection(*it) )
				{
					// Exception
					// Break twice
					goto cleanUp;
				}
				it = queue.erase( it );
				continue;
			}
#ifdef _DEBUG
			std::cout << "Sending its order to the client " << *it << " waiting in the queue line succeeded.\n";
#endif
			clientSocket.pend( );
			++it;
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
				std::cerr << "Failed to send a copy of the issued ticket to main server.\n";
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
			if ( 1 == status[Status::AWAITING_POPULATION] && false == socketToMainServer->isPending() )
			{
#ifdef _DEBUG
				std::cout << "Waiting for the main server's answer how many clients are there.\n";
#endif
				tolerance = 0;
				while ( -1 == socketToMainServer->receiveOverlapped( )
						&& ERROR_IO_PENDING != WSAGetLastError( ) )
				{
					// Exception
					std::cerr << "Failed to pend reception from the main server.\n";
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
			const Index nextCandidateClientIdx = candidateS.front( );
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
			const Index evIdx = (Index)event.events[ i ].lpCompletionKey;
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