#include "pch.h"
#include "Room.h"
#include "Client.h"
#include "Tetrimino.h"

const uint32_t CLIENT_CAPACITY = 100;
const uint32_t ROOM_CAPACITY = 25;
const uint32_t LOBBY_CAPACITY = 100;
const ClientIndex LISTENER_IDX = CLIENT_CAPACITY;

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

	{
		WSAData w;
		WSAStartup( MAKEWORD(2,2), &w );
	}

	SYSTEM_INFO sysInfo;
	GetSystemInfo( &sysInfo );
	IOCP iocp( 2*sysInfo.dwNumberOfProcessors + 2 );

	////
	//
	////

	Socket listener( Socket::Type::TCP );
	{
		bool result = true;
		if ( -1 == listener.bind(EndPoint(MAIN_SERVER_IP_ADDRESS, MAIN_SERVER_PORT)) )
		{
			// Exception
			std::cerr << "FATAL: Failed to bind listener.\n";
			result = false;
		}
		//궁금: 항상 받기 걸어두기가 가능하면 이게 필요하다.
		//else if ( const DWORD rcvBufSize = 0;
		//	0 != ::setsockopt(listener.handle(), SOL_SOCKET, SO_RCVBUF, (char*)&rcvBufSize, sizeof(DWORD)) )
		//{
		//	// Exception
		//	std::cerr << "Failed to set receiving buffer size to zero for the listener.\n";
		//}
		//궁금: 8KB 받기 버퍼에게 오류를 일으키는 것 같다.
		//else if ( const DWORD SND_BUF_SIZ = 0;
		//		0 != ::setsockopt(listener.handle(), SOL_SOCKET, SO_SNDBUF, (char*)&SND_BUF_SIZ, sizeof(DWORD)) )
		//{
		//	// Exception
		//	std::cerr << "FATAL: Failed to set sending buffer size to zero for the listener.\n";
		//	result = false;
		//}
		else if ( -1 == iocp.add(listener.handle(), LISTENER_IDX) )
		{
			// Exception
			std::cerr << "FATAL: Failed to add listener into IOCP.\n";
			result = false;
		}
		else
		{
			listener.listen( );
		}

		if ( false == result )
		{
			listener.close( );
			WSACleanup( );
			return -1;
		}
	}

	////
	//
	////

	std::vector<Client> clients;
	clients.reserve( CLIENT_CAPACITY );
	container::IteratoredQueue<ClientIndex> candidates;
	candidates.reserve( CLIENT_CAPACITY );
	for ( uint32_t i = 0; i != CLIENT_CAPACITY; ++i )
	{
		Client& client = clients.emplace_back(Socket::Type::TCP, i);
		const Socket& socket = client.socket();
		if ( -1 == iocp.add(socket.handle(), i) )
		{
			std::cerr << "FATAL: Failed to add Client " << i << " into IOCP.\n";
			listener.close( );
			WSACleanup( );
			clients.clear( );
			return -1;
		}
		candidates.emplace_back( i );
	}
	{
		const int result = listener.acceptOverlapped(clients.at(candidates.front()).socket());
		if ( FALSE == result )
		{
			// Exception
			std::cerr << "FATAL: Failed to accept.\n";
			listener.close( );
			clients.clear( );
			WSACleanup( );
			return -1;
		}
		else if ( -1 == result )
		{
			// Exception
			std::cerr << "FATAL: Failed to get AcceptEx(...).\n";
			listener.close( );
			clients.clear( );
			WSACleanup( );
			return -1;
		}
	}

	////
	//
	////

	// NOTE: std::vector with cache runs faster than std::list.
	std::vector<ClientIndex> lobby;
	lobby.reserve( LOBBY_CAPACITY );
	std::unordered_map<HashedKey, Room> rooms;
	rooms.reserve( ROOM_CAPACITY );
	auto forceDisconnection = [ &clients, &iocp, &candidates, &lobby, &rooms ]( const ClientIndex index ) -> bool
	{
		bool result = true;
		Client& client = clients[index];
		if ( Client::State::IN_LOBBY == client.state() )
		{
			for ( auto it = lobby.cbegin(); lobby.cend() != it; ++it )
			{
				if ( index == *it )
				{
					lobby.erase( it );
					break;
				}
			}
		}
		else if ( const auto it = rooms.find(client.roomID());
				 rooms.end() != it )
		{
			if ( 0 == it->second.leave(index) )
			{
#ifdef _DEBUG
				std::cout << "Room "
					<< it->first << " has been destructed.\n";
#endif
				rooms.erase( it );
			}
		}
		client.reset( false );
		const Socket& socket = client.socket();
		if ( -1 == iocp.add(socket.handle(), index) )
		{
			// Exception
			std::cerr << "FATAL: Failed to add Client " << index << " into IOCP.\n";
			result = false;
			return result;
		}
		candidates.emplace_back( index );
		std::cout << "Forced to disconnect Client " << index << ". (Now "
			<< CLIENT_CAPACITY-candidates.size() << "/" << CLIENT_CAPACITY << " connections.)\n";
		return result;
	};
	bool wasBoatful = false;
	ClientIndex queueServerIdx = -1;
	std::cout << "###############\n### MAIN SERVER\n###############\n\nWhat would the queue server say?" << std::endl;
	std::string sign;
	std::cin >> sign;
	const HashedKey encryptedSign = ::util::hash::Digest( sign.data(), (uint8_t)sign.size() );
	std::cout << "Ready.\n";
	std::vector<Ticket> tickets;
	IOCPEvent event;

	////
	//
	////

	while ( true == IsWorking )
	{
		iocp.wait( event, 100 );
		for ( uint32_t i = 0u; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[i];
			////
			// When event comes from listener,
			////
			if ( LISTENER_IDX == (ClientIndex)ev.lpCompletionKey )
			{
				const IOType cmpl = listener.completedIO(ev.lpOverlapped,
														 ev.dwNumberOfBytesTransferred);
#ifdef _DEBUG
				if ( IOType::ACCEPT != cmpl )
				{
					__debugbreak( );
				}
#endif
				const ClientIndex candidate = candidates.front();
				Socket& candidateClientSocket = clients[candidate].socket();
				if ( -1 == candidateClientSocket.updateAcceptContext(listener) )
				{
					// Exception
					std::cerr << "FATAL: Failed to update acception context of Candidate " << candidate << ".\n";
					// Break twice
					goto cleanUp;
				}
				// When accepting successfully,
				Socket& clientSocket = candidateClientSocket;
				const ClientIndex clientIdx = candidate;
				if ( -1 == clientSocket.receiveOverlapped() )
				{
					// Exception
					std::cerr << "WARNING: Failed to receive from a new client " << clientIdx << ".\n";
					if ( false == forceDisconnection(clientIdx) )
					{
						// Break twice
						goto cleanUp;
					}
				}
				else
				{
					candidates.pop_front( );
#ifdef _DEBUG
					std::cout << "A new client " << clientIdx << " joined. (Now "
						<< CLIENT_CAPACITY-candidates.size() << "/" << CLIENT_CAPACITY << " connections.)\n";
#endif
				}

				// Reloading the next candidate.
				if ( 0 < candidates.size() )
				{
					const ClientIndex nextCandidate = candidates.front();
					if ( FALSE == listener.acceptOverlapped(clients[nextCandidate].socket()) )
					{
						// Exception
						std::cerr << "FATAL: Failed to accept.\n";
						goto cleanUp;
					}
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
			// When event comes from the queue server,
			////
			else if ( queueServerIdx == (ClientIndex)ev.lpCompletionKey )
			{
				if ( true == candidates.contains(queueServerIdx) )
				{
					continue;
				}
				Client& queueServer = clients[queueServerIdx];
				Socket& socketToQueueServer = queueServer.socket();
				const IOType cmpl = socketToQueueServer.completedIO(ev.lpOverlapped,
																	ev.dwNumberOfBytesTransferred);
				if ( IOType::DISCONNECT == cmpl )
				{
					queueServer.reset( );
					candidates.emplace_back( queueServerIdx );
					// Reset
					queueServerIdx = -1;
#ifdef _DEBUG
					std::cerr << "WARNING: The queue server disconnected. (Now "
						<< CLIENT_CAPACITY-candidates.size( ) << '/' << CLIENT_CAPACITY << " connections.)\n";
#endif
				}
				else if ( 0 == ev.dwNumberOfBytesTransferred )
				{
					const int result = socketToQueueServer.disconnectOverlapped();
					if ( FALSE == result )
					{
						// Exception
						std::cerr << "WARNING: Failed to disconnect the queue server.\n";
						if ( false == forceDisconnection(queueServerIdx) )
						{
							// Break twice
							goto cleanUp;
						}
						// Reset
						queueServerIdx = -1;
					}
					else if ( -1 == result )
					{
						std::cerr << "FATAL: Failed to get DisconnectEx(...) for the queue server, Client "
							<< queueServerIdx << ".\n";
						// Break twice
						goto cleanUp;
					}
				}
				else
				{
					switch ( cmpl )
					{
						case IOType::RECEIVE:
						{
// TODO: getline
							const char* const rcvBuf = socketToQueueServer.receivingBuffer();
							const std::string_view strView( rcvBuf );
							// When the queue server asked how many clients keep connecting,
// 궁금: receive 쪼개서 받을 수 있으면 이렇게 안 해도 될텐데.
							if ( const size_t pos = strView.find(TAG_POPULATION);
								std::string_view::npos != pos )
							{
#ifdef _DEBUG
								std::cout << "Population has been asked by the queue server.\n";
#endif
								const uint32_t pop = CLIENT_CAPACITY-candidates.size();
								Packet packet;
								packet.pack( TAG_POPULATION, pop );
								if ( -1 == socketToQueueServer.sendOverlapped(packet) )
								{
									// Exception
									std::cerr << "WARNING: Failed to receive population\n";
									if ( false == forceDisconnection(queueServerIdx) )
									{
										// Break twice
										goto cleanUp;
									}
									// Reset
									queueServerIdx = -1;
								}
								else
								{
#ifdef _DEBUG
									std::cout << "Population has been told: " << pop << ".\n";
#endif
								}
							}
							uint32_t off = 0;
							// When having received copies of the issued ticket,
							constexpr uint8_t TAG_TICKET_LEN = ::util::hash::Measure(TAG_TICKET);
							while ( true )
							{
								const size_t tagPos = strView.find( TAG_TICKET, off );
								if ( std::string_view::npos == tagPos )
								{
									break;
								}
 								const uint32_t beginPos = (uint32_t)tagPos + TAG_TICKET_LEN;
								off += sizeof(Ticket);
								const Ticket id = ::ntohl(*(Ticket*)&rcvBuf[beginPos]);
								tickets.emplace_back( id );
#ifdef _DEBUG
								std::cout << "A copy of the issued ticket arrived: " << id << ".\n";
#endif
							}
							//TODO: 끊임없이 받기 걸어두고 싶다.
							if ( -1 == socketToQueueServer.receiveOverlapped() )
							{
								// Exception
								std::cerr << "WARNING: Failed to receive from the queue server.\n";
								if ( false == forceDisconnection(queueServerIdx) )
								{
									// Break twice
									goto cleanUp;
								}
								// Reset
								queueServerIdx = -1;
							}
							break;
						}
						case IOType::SEND:
							if ( -1 == socketToQueueServer.receiveOverlapped() )
							{
								// Exception
								std::cerr << "WARNING: Failed to receive from the queue server\n";
								if ( false == forceDisconnection(queueServerIdx) )
								{
									// Break twice
									goto cleanUp;
								}
								// Reset
								queueServerIdx = -1;
								continue;
							}
							break;
						default:
#ifdef _DEBUG
							__debugbreak( );
#else
							__assume(0);
#endif
							break;
					}
				}
			}
			////
			// When event comes from clients,
			////
			else
			{
				const ClientIndex clientIdx = (ClientIndex)ev.lpCompletionKey;
				if ( true == candidates.contains(clientIdx) )
				{
					continue;
				}
				Client& client = clients[clientIdx];
				Socket& clientSocket = client.socket();
				const IOType cmpl =	clientSocket.completedIO(ev.lpOverlapped,
															 ev.dwNumberOfBytesTransferred);
				if ( IOType::DISCONNECT == cmpl )
				{
					if ( Client::State::IN_LOBBY == client.state() )
					{
						for ( auto it = lobby.cbegin(); lobby.cend() != it; ++it )
						{
							if ( clientIdx == *it )
							{
								lobby.erase( it );
								break;
							}
						}
					}
					else if ( const auto it = rooms.find(client.roomID());
								rooms.end() != it )
					{
						if ( 0 == it->second.leave(clientIdx) )
						{
#ifdef _DEBUG
							std::cout << "Room "
								<< it->first << " has been destructed.\n";
#endif
							rooms.erase( it );
						}
					}
					client.reset( );
					candidates.emplace_back( clientIdx );
#ifdef _DEBUG
					std::cout << "Client " << clientIdx << " left. (Now "
						<< CLIENT_CAPACITY-candidates.size( ) << "/" << CLIENT_CAPACITY << " connections.)\n";
#endif
				}
				else if ( 0 == ev.dwNumberOfBytesTransferred )
				{
					const int res = clientSocket.disconnectOverlapped( );
					if ( FALSE == res )
					{
						// Exception
						std::cerr << "WARNING: Failed to disconnect Client " << clientIdx << ".\n";
						if ( false == forceDisconnection(clientIdx) )
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
				}
				else
				{
					if ( Client::State::UNVERIFIED == client.state() )
					{
						if ( IOType::RECEIVE == cmpl )
						{
							const char* const rcvBuf = clientSocket.receivingBuffer();
							constexpr uint8_t TAG_TICKET_LEN = ::util::hash::Measure(TAG_TICKET);
							const Ticket ticket = ::ntohl(*(Ticket*)&rcvBuf[TAG_TICKET_LEN]);
							
							auto it = tickets.cbegin();
							while ( tickets.cend() != it )
							{
								// When having a copy of the ticket received from this client,
								if ( ticket == *it )
								{
									break;
								}
								else
								{
									++it;
								}
							}
							if ( tickets.cend() != it )
							{
								client.holdTicket( ticket );
								tickets.erase( it );
//TODO: 닉네임
								std::string nickname( "nickname" );
								nickname += std::to_string( clientIdx );
								{
									Packet packet;
									packet.pack( TAG_MY_NICKNAME, nickname );
									if ( -1 == clientSocket.sendOverlapped(packet) )
									{
										// Exception
										std::cerr << "Failed to acknowledge Client " << clientIdx << ".\n";
										if ( false == forceDisconnection(clientIdx) )
										{
											// Break twice
											goto cleanUp;
										}
										continue;
									}
								}
								client.setNickname( nickname );
								client.setState( Client::State::IN_LOBBY );
								lobby.emplace_back( clientIdx );
								if ( -1 == clientSocket.receiveOverlapped() )
								{
									// Exception
									std::cerr << "WARNING: Failed to receive from Client " << clientIdx << ".\n";
									if ( false == forceDisconnection(clientIdx) )
									{
										// Break twice
										goto cleanUp;
									}
									continue;
								}
							}
							// When the queue server(now as a client) asked how many clients keep connecting,
							else if ( encryptedSign == ::ntohl(*(HashedKey*)rcvBuf) )
							{
								queueServerIdx = clientIdx;
								client.setState( Client::State::AS_QUEUE_SERVER );
#ifdef _DEBUG
								std::cout << "Population has been asked by the queue server.\n";
#endif
								Packet packet;
								const uint32_t pop = CLIENT_CAPACITY-candidates.size();
								packet.pack( TAG_POPULATION, pop );
								Socket& queueServerSocket = clientSocket;
								if ( -1 == queueServerSocket.sendOverlapped(packet) )
								{
									// Exception
									std::cerr << "WARNING: Failed to send population.\n";
									if ( false == forceDisconnection(queueServerIdx) )
									{
										// Break twice
										goto cleanUp;
									}
									continue;
								}
#ifdef _DEBUG
								std::cout << "Population has been told: " << pop << ".\n";
#endif
								if ( -1 == clientSocket.receiveOverlapped() )
								{
									// Exception
									std::cerr << "WARNING: Failed to receive from Client " << clientIdx << ".\n";
									if ( false == forceDisconnection(clientIdx) )
									{
										// Break twice
										goto cleanUp;
									}
									continue;
								}
							}
							// Exception
							// When not having the copy of the ticket or
							// the sign doesn't match the one from the queue server,
							else
							{
								if ( -1 == queueServerIdx )
								{
									std::cerr << "WARNING: The sign doesn't match the one from the queue server.\n";
								}
								else
								{
									std::cerr << "WARNING: Tried to disconnect Client " << clientIdx << " with an invalid ticket.\n";
								}

								const int result = clientSocket.disconnectOverlapped( );
								if ( FALSE == result )
								{
									// Exception
									std::cerr << "WARNING: Failed to disconnect Client " << clientIdx << ".\n";
									if ( false == forceDisconnection(clientIdx) )
									{
										// Break twice
										goto cleanUp;
									}
								}
								else if ( -1 == result )
								{
									std::cerr << "FATAL: Failed to get DisconnectEx(...) for Client " << clientIdx << ".\n";
									// Break twice
									goto cleanUp;
								}
							}
						}
					}
					else
					{
						std::vector<ClientIndex> failedIndices( client.work(cmpl, clients, lobby, rooms) );
						for ( const ClientIndex idx : failedIndices )
						{
							if ( false == forceDisconnection(idx) )
							{
								// Break three times
								goto cleanUp;
							}
						}
					}
				}
			}
		}

		if ( 0 < rooms.size() )
		{
			auto it = rooms.begin();
			auto it2 = rooms.begin();
			++it2;
			while ( true )
			{
				const HashedKey roomID = it->first;
				std::vector<ClientIndex> failedIndices( it->second.update(clients) );
				for ( const ClientIndex index : failedIndices )
				{
					if ( false == forceDisconnection(index) )
					{
						// Break three times
						goto cleanUp;
					}
				}
				if ( rooms.end() != rooms.find(roomID) )
				{
					failedIndices = it->second.notify(clients);
					for ( const ClientIndex index : failedIndices )
					{
						if ( false == forceDisconnection(index) )
						{
							// Break three times
							goto cleanUp;
						}
					}
				}
				it = it2;
				if ( rooms.end() == it )
				{
					break;
				}
				++it2;
			}
		}

		if ( true == wasBoatful && 0 < candidates.size() )
		{
			wasBoatful = false;
			const ClientIndex nextCandidateClientIdx = candidates.front();
			if ( FALSE == listener.acceptOverlapped(clients[nextCandidateClientIdx].socket()) )
			{
				// Exception
				std::cerr << "FATAL: Overlapped acceptEx failed.\n";
				break;
			}
		}
	}

cleanUp:
	//!IMPORTANT: Must release all the overlapped I/O in hand before closing the server.
	//	 		  Otherwise overlapped I/O still runs on O/S background.
	std::cout << "Server gets closed.\n";
	listener.close( );
	uint32_t areClientsPending = 0;
	for ( auto& client : clients )
	{
		Socket& socket = client.socket();
		socket.close( );
		if ( true == socket.isPending() )
		{
			++areClientsPending;
		}
	}
	while ( 0 < areClientsPending ||
		   true == listener.isPending() )
	{
		IOCPEvent event;
		iocp.wait( event, 100 );
		for ( uint32_t i = 0; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[i];
			const ClientIndex evIdx = (ClientIndex)ev.lpCompletionKey;
			// When event comes from listener,
			if ( LISTENER_IDX == evIdx )
			{
				listener.completedIO( ev.lpOverlapped,
									 ev.dwNumberOfBytesTransferred );
			}
			// When event comes from TCP,
			else
			{
				Socket& socket = clients[evIdx].socket();
				socket.completedIO( ev.lpOverlapped,
								   ev.dwNumberOfBytesTransferred );
				--areClientsPending;
			}
		}
	}
	clients.clear( );
	std::cout << "Server has been closed successfully." << std::endl;

	WSACleanup( );

	return 0;
}