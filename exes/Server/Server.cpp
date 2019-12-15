#include "pch.h"
#include "Room.h"
#include "Client.h"
#include "Tetrimino.h"

//TODO: 접속 종료한 지 얼마 안 되었을 때 바로 접속 가능하게

//DOING: 로직

const uint32_t CLIENT_CAPACITY = 2u;
const uint32_t ROOM_CAPACITY = 100u;
const uint16_t LISTENER_PORT = MAIN_SERVER_PORT;

volatile bool IsWorking = true;
void ProcessSignal( int signal )
{
	if ( SIGINT == signal )
	{
		IsWorking = false;
	}
}

int main()
{
	signal( SIGINT, &ProcessSignal );

	WSAData w;
	WSAStartup( MAKEWORD(2,2), &w );

	SYSTEM_INFO sysInfo;
	GetSystemInfo( &sysInfo );
	IOCP iocp( 2*sysInfo.dwNumberOfProcessors + 2 );

	Socket listener( Socket::Type::TCP_LISTENER );
	{
		char ipAddress[] = "192.168.219.102";
		if ( -1 == listener.bind(EndPoint(ipAddress, LISTENER_PORT)) )
		{
			// Exception
			std::cerr << "FATAL: Failed to bind listener.\n";
			listener.close( );
			WSACleanup( );
			return -1;
		}
	}
	listener.listen( );
	const ClientIndex LISTENER_IDX = CLIENT_CAPACITY;
	if ( -1 == iocp.add(listener, LISTENER_IDX) )
	{
		// Exception
		std::cerr << "FATAL: Failed to add listener into IOCP.\n";
		listener.close( );
		WSACleanup( );
		return -1;
	}
	std::vector< Client > clientS;
	clientS.reserve( CLIENT_CAPACITY );
	// Must use push_back(or emplace_back), front and pop_front only, like std::queue.
	container::IteratoredQueue< ClientIndex > candidatesIndices;
	candidatesIndices.reserve( CLIENT_CAPACITY );
	for ( uint32_t i = 0; i != CLIENT_CAPACITY; ++i )
	{
		clientS.emplace_back( Socket::Type::TCP, i );
		if ( -1 == iocp.add(clientS.at(i).socket(), i) )
		{
			std::cerr << "FATAL: Failed to add Client " << i << " into IOCP.\n";
			listener.close( );
			WSACleanup( );
			clientS.clear( );
			return -1;
		}
		candidatesIndices.emplace_back( i );
	}
	const int result = listener.acceptOverlapped( clientS.at(candidatesIndices.front()).socket() );
	if ( 0 == result && ERROR_IO_PENDING != WSAGetLastError() )
	{
		// Exception
		listener.close( );
		clientS.clear( );
		WSACleanup( );
		std::cerr << "FATAL: Failed to accept.\n";
		return -1;
	}
	else if ( -1 == result )
	{
		// Exception
		listener.close( );
		clientS.clear( );
		WSACleanup( );
		std::cerr << "FATAL: Failed to get AcceptEx(...).\n";
		return -1;
	}
	listener.pend( );
	std::unordered_map< HashedKey, Room > roomS;
	roomS.reserve( ROOM_CAPACITY );
	auto forceDisconnection = [ &clientS, &iocp, &candidatesIndices, &roomS ]( const ClientIndex idx )->int
	{
		Client& client = clientS[ idx ];
		client.setSocket( Socket::Type::TCP, Socket::CompletedWork::DISCONNECT );
		candidatesIndices.emplace_back( idx );
		std::cout << "Forced to disconnect Client " << idx << ". (Now "
			<< CLIENT_CAPACITY-candidatesIndices.size() << "/" << CLIENT_CAPACITY << " connections.)\n";
		if ( -1 == iocp.add(client.socket(), idx) )
		{
			// Exception
			std::cerr << "FATAL: Failed to add Client " << idx << " into IOCP.\n";
			return -1;
		}

		if ( const auto it = roomS.find(client.roomID()); roomS.end() != it )
		{
			it->second.leave( idx );
		}
		client.reset( );
		return 0;
	};

	std::cout << "##########\n### MAIN SERVER\n##########\n\nWhat is today's salt, Sir?" << std::endl;
	std::string todaysSalt;
	std::cin >> todaysSalt;
	std::cout << "Ready.\n";
	const HashedKey encryptedSalt = ::util::hash::Digest( todaysSalt.data(), (uint8_t)todaysSalt.size() );
	bool wasBoatful = false;
	ClientIndex queueServerIdx = -1;
	std::unordered_set< Ticket > ticketS;
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
			if ( LISTENER_IDX == (ClientIndex)ev.lpCompletionKey && 0 < candidatesIndices.size() )
			{
				listener.pend( false );
				const ClientIndex candidateClientIdx = candidatesIndices.front( );
				Socket& candidateClientSocket = clientS[ candidateClientIdx ].socket( );
				if ( -1 == candidateClientSocket.updateAcceptContext(listener) )
				{
					// Exception
					// Break twice
					goto cleanUp;
				}
				// When acception is successful,
				Socket& clientSocket = candidateClientSocket;
				const ClientIndex clientIdx = candidateClientIdx;
				if ( -1 == clientSocket.receiveOverlapped() )
				{
					// Exception
					std::cerr << "Failed to pend reception from a new client " << clientIdx << ".\n";
					if ( -1 == forceDisconnection(clientIdx) )
					{
						// Break twice
						goto cleanUp;
					}
				}
				else
				{
					candidatesIndices.pop_front( );
					clientSocket.pend( );
#ifdef _DEBUG
					std::cout << "A new client " << clientIdx << " joined. (Now "
						<< CLIENT_CAPACITY-candidatesIndices.size() << "/" << CLIENT_CAPACITY << " connections.)\n";
#endif
				}

				// Reloading the next candidate.
				if ( 0 < candidatesIndices.size() )
				{
					const ClientIndex nextCandidateIdx = candidatesIndices.front( );
					if ( 0 == listener.acceptOverlapped(clientS[nextCandidateIdx].socket())
						 && ERROR_IO_PENDING != WSAGetLastError() )
					{
						// Exception
						std::cerr << "Failed to accept.\n";
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
			else if ( queueServerIdx == (ClientIndex)ev.lpCompletionKey )
			{
				Socket& socketToQueueServer = clientS[ queueServerIdx ].socket( );
				socketToQueueServer.pend( false );
				if ( Socket::CompletedWork::DISCONNECT == socketToQueueServer.completedWork() )
				{
					const int res = candidatesIndices.contains( queueServerIdx );
					if ( -1 == res )
					{
						std::cerr << "WARNING: Client " << queueServerIdx << " is already candidate.\n";
					}
					else if ( 0 == res )
					{
						candidatesIndices.emplace_back( queueServerIdx );
						// Reset
						queueServerIdx = -1;
#ifdef _DEBUG
						std::cerr << "WARNING: The queue server disconnected. (Now "
							<< CLIENT_CAPACITY-candidatesIndices.size( ) << '/' << CLIENT_CAPACITY << " connections.)\n";
#endif
					}
				}
				// 0 in disconnection, <0 in error.
				else if ( ev.dwNumberOfBytesTransferred <= 0 )
				{
					if ( ev.dwNumberOfBytesTransferred < 0 )
					{
						std::cerr << "WARNING: Failed to send to/receive from the queue server.\n";
					}

					const int res = socketToQueueServer.disconnectOverlapped( );
					if ( 0 == res && ERROR_IO_PENDING != WSAGetLastError() )
					{
						// Exception
						std::cerr << "Failed to disconnect the queue server.\n";
						if ( -1 == forceDisconnection(queueServerIdx) )
						{
							// Break twice
							goto cleanUp;
						}
						// Reset
						queueServerIdx = -1;
					}
					else if ( -1 == res )
					{
						std::cerr << "FATAL: Failed to get DisconnectEx(...) for the queue server, Client "
							<< queueServerIdx << ".\n";
						// Break twice
						goto cleanUp;
					}
					else
					{
						socketToQueueServer.pend( );
					}
				}
				else
				{
					const Socket::CompletedWork cmpl = socketToQueueServer.completedWork( );
					if ( Socket::CompletedWork::RECEIVE == cmpl )
					{
// TODO: getline
						const char* const rcvBuf = socketToQueueServer.receivingBuffer();
						const std::string_view strView( rcvBuf );
						// When the queue server asked how many clients keep connecting,
						if ( const size_t pos = strView.find(TAG_POPULATION);
							std::string_view::npos != pos )
						{
#ifdef _DEBUG
							std::cout << "Population has been asked by the queue server.\n";
#endif
							const uint32_t pop = CLIENT_CAPACITY-candidatesIndices.size();
							Packet packet;
							packet.pack( TAG_POPULATION, pop );
							if ( -1 == socketToQueueServer.sendOverlapped(packet) )
							{
								// Exception
								std::cerr << "Failed to pend sending population\n";
								if ( -1 == forceDisconnection(queueServerIdx) )
								{
									// Break twice
									goto cleanUp;
								}
								// Reset
								queueServerIdx = -1;
							}
							else
							{
								socketToQueueServer.pend( );
#ifdef _DEBUG
								std::cout << "Population has been told: " << pop << ".\n";
#endif
							}
						}
						else
						{
							if ( -1 == socketToQueueServer.receiveOverlapped() )
							{
								// Exception
								std::cerr << "Failed to pend reception from the queue server\n";
								if ( -1 == forceDisconnection(queueServerIdx) )
								{
									// Break twice
									goto cleanUp;
								}
								// Reset
								queueServerIdx = -1;
							}
							else
							{
								socketToQueueServer.pend( );
							}
						}
						
						size_t off = 0u;
						// When having received copies of the issued ticket,
						while ( true )
						{
							const size_t tagPos = strView.find( TAG_TICKET, off );
							if ( std::string_view::npos == tagPos )
							{
								break;
							}
							constexpr uint8_t TAG_TICKET_LEN = ::util::hash::Measure( TAG_TICKET );
 							const uint32_t beginPos = (uint32_t)tagPos + TAG_TICKET_LEN;
							off += sizeof(Ticket);
							const Ticket id = ::ntohl(*(Ticket*)&rcvBuf[beginPos]);
							ticketS.emplace( id );
#ifdef _DEBUG
							std::cout << "A copy of the issued ticket arrived: " << id << ".\n";
#endif
						}
					}
					else if ( Socket::CompletedWork::SEND == cmpl )
					{
// NOTE: Here is BLACK SPOT.
// Frequently-happening error: 10053, 10054.
						if ( -1 == socketToQueueServer.receiveOverlapped() )
						{
							// Exception
							std::cerr << "Failed to pend reception from the queue server\n";
							if ( -1 == forceDisconnection(queueServerIdx) )
							{
								// Break twice
								goto cleanUp;
							}
							// Reset
							queueServerIdx = -1;
							continue;
						}
						socketToQueueServer.pend( );
					}
					// Exception
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
			////
			// When event comes from clients,
			////
			else
			{
				const ClientIndex clientIdx = (ClientIndex)ev.lpCompletionKey;
				Client& client = clientS[ clientIdx ];
				Socket& clientSocket = client.socket( );
				clientSocket.pend( false );
				const Socket::CompletedWork cmpl = clientSocket.completedWork( );
				if ( Socket::CompletedWork::DISCONNECT == cmpl )
				{
					const int res = candidatesIndices.contains( clientIdx );
					if ( -1 == res )
					{
						std::cerr << "WARNING: Client " << clientIdx << " is already candidate.\n";
					}
					else if ( 0 == res )
					{
						if ( const auto it = roomS.find(client.roomID()); roomS.end() != it )
						{
							it->second.leave( clientIdx );
						}
						client.reset( );
						candidatesIndices.emplace_back( clientIdx );
#ifdef _DEBUG
						std::cout << "Client " << clientIdx << " left. (Now "
							<< CLIENT_CAPACITY-candidatesIndices.size( ) << "/" << CLIENT_CAPACITY << " connections.)\n";
#endif
					}
				}
				// 0 in disconnection, <0 in error.
				else if ( ev.dwNumberOfBytesTransferred <= 0 )
				{
					if ( ev.dwNumberOfBytesTransferred < 0 )
					{
						std::cerr << "WARNING: Failed to send to/receive from Client " << clientIdx << ".\n";
					}

					const int res = clientSocket.disconnectOverlapped( );
					if ( 0 == res && ERROR_IO_PENDING != WSAGetLastError() )
					{
						// Exception
						std::cerr << "Failed to disconnect Client " << clientIdx << ".\n";
						if ( -1 == forceDisconnection(clientIdx) )
						{
							// Break twice
							goto cleanUp;
						}
					}
					else if ( -1 == res )
					{
						std::cerr << "FATAL: Failed to get DisconnectEx(...) for Client " << clientIdx << ".\n";
						// Break twice
						goto cleanUp;
					}
					else
					{
						clientSocket.pend( );
					}
				}
				else
				{
					if ( Client::State::UNVERIFIED == client.state() )
					{
						if ( Socket::CompletedWork::RECEIVE == cmpl )
						{
							const char* const rcvBuf = clientSocket.receivingBuffer( );
							// When having a copy of the ticket received from this client,
							constexpr uint8_t TAG_TICKET_LEN = ::util::hash::Measure( TAG_TICKET );
							if ( const Ticket ticket = ::ntohl(*(Ticket*)&rcvBuf[TAG_TICKET_LEN]);
								ticketS.cend() != ticketS.find(ticket) )
							{
								client.holdTicket( ticket );
								ticketS.erase( ticket );
//TODO: 닉네임
								Packet packet;
								std::string nickname( "nickname01" );
								packet.pack( TAG_MY_NICKNAME, nickname );
								if ( -1 == clientSocket.sendOverlapped(packet) )
								{
									// Exception
									std::cerr << "Failed to acknowledge Client " << clientIdx << ".\n";
									if ( -1 == forceDisconnection(clientIdx) )
									{
										// Break twice
										goto cleanUp;
									}
								}
								clientSocket.pend( );
							}
							// When the queue server as a client asked how many clients keep connecting,
							else if ( encryptedSalt == ::ntohl(*(HashedKey*)rcvBuf) )
							{
								queueServerIdx = clientIdx;
#ifdef _DEBUG
								std::cout << "Population has been asked by the queue server.\n";
#endif
								// Letting the queue server know the population here
								// makes the queue server possible
								// to scale up/down the virtual capacity of the main server elastically and independently.
								Packet packet;
								const uint32_t pop = CLIENT_CAPACITY-candidatesIndices.size();
								packet.pack( TAG_POPULATION, pop );
								Socket& queueServerSocket = clientSocket;
								if ( -1 == queueServerSocket.sendOverlapped(packet) )
								{
									// Exception
									std::cerr << "Failed to pend sending population\n";
									if ( -1 == forceDisconnection(queueServerIdx) )
									{
										// Break twice
										goto cleanUp;
									}
									continue;
								}
								queueServerSocket.pend( );
#ifdef _DEBUG
								std::cout << "Population has been told: " << pop << ".\n";
#endif
							}
							// When not having a copy of the ticket received from this client or
							// receiving the unmatching salt from the queue server,
							else
							{
								if ( -1 == queueServerIdx )
								{
									std::cerr << "WARNING: Unmatching salt from the queue server.\n";
								}
								else
								{
									std::cerr << "WARNING: Tried to disconnect Client " << clientIdx << " with an invalid ticket.\n";
								}

								const int res = clientSocket.disconnectOverlapped( );
								if ( 0==res && ERROR_IO_PENDING!=WSAGetLastError() )
								{
									// Exception
									std::cerr << "Failed to disconnect Client " << clientIdx << ".\n";
									if ( -1 == forceDisconnection(clientIdx) )
									{
										// Break twice
										goto cleanUp;
									}
								}
								else if ( -1 == res )
								{
									std::cerr << "FATAL: Failed to get DisconnectEx(...) for Client " << clientIdx << ".\n";
									// Break twice
									goto cleanUp;
								}
								else
								{
									clientSocket.pend( );
								}
							}
						}
						else if ( Socket::CompletedWork::SEND == cmpl )
						{
							client.setState( Client::State::IN_LOBBY );
							if ( -1 == clientSocket.receiveOverlapped() )
							{
								// Exception
								std::cerr << "Failed to pend reception from Client " << clientIdx << ".\n";
								if ( -1 == forceDisconnection(clientIdx) )
								{
									// Break twice
									goto cleanUp;
								}
								continue;
							}
							clientSocket.pend( );
						}
						// Exception
						else
						{
#ifdef _DEBUG
								__debugbreak( );
#else
								__assume(0);
#endif
						}
					}
					else
					{
						if ( false == client.complete(roomS) )
						{
							if ( -1 == forceDisconnection(clientIdx) )
							{
								// Break twice
								goto cleanUp;
							}
						}
					}
				}
			}
		}

		for ( auto it = roomS.begin(); roomS.end() != it; ++it )
		{
			std::forward_list< ClientIndex > failedIndices( it->second.update() );
			for ( const ClientIndex index : failedIndices )
			{
				if ( -1 == forceDisconnection(index) )
				{
					// Break three times
					goto cleanUp;
				}
			}

			failedIndices = it->second.notify( clientS );
			for ( const ClientIndex index : failedIndices )
			{
				if ( -1 == forceDisconnection(index) )
				{
					// Break three times
					goto cleanUp;
				}
			}
		}

		if ( true == wasBoatful && 0 < candidatesIndices.size() )
		{
			wasBoatful = false;
			const ClientIndex nextCandidateClientIdx = candidatesIndices.front( );
			if ( 0 == listener.acceptOverlapped(clientS[nextCandidateClientIdx].socket())
				 && ERROR_IO_PENDING != WSAGetLastError() )
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
	for ( auto& client : clientS )
	{
		Socket& socket = client.socket( );
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
			const ClientIndex evIdx = (ClientIndex)event.events[ i ].lpCompletionKey;
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