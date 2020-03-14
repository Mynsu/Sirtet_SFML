#include "pch.h"
#include "Room.h"
#include "Client.h"
#include "Tetrimino.h"

const uint16_t BACKLOG_SIZ = CLIENT_CAPACITY/2; // NOTE: Assure this be not zero.
const uint16_t ROOM_CAPACITY = 25;
const uint16_t LOBBY_CAPACITY = 100;
const ClientIndex LISTENER_IDX = CLIENT_CAPACITY;
const Clock::duration TCP_TIMED_WAIT_DELAY_SEC = std::chrono::seconds(30);
const Clock::duration TICKET_SUBMISSION_TIME_LIMIT = std::chrono::seconds(2);

volatile bool IsWorking = true;
void ProcessSignal( int signal )
{
	if ( SIGINT == signal )
	{
		IsWorking = false;
	}
}

int main( )
{
	signal( SIGINT, &ProcessSignal );

	{
		WSAData w;
		WSAStartup( MAKEWORD(2,2), &w );
	}

	SYSTEM_INFO sysInfo;
	GetSystemInfo( &sysInfo );
	IOCP iocp( 2*sysInfo.dwNumberOfProcessors + 2 );

	Socket listener( Socket::Type::TCP );
	{
		bool result = true;
		if ( -1 == listener.bind(EndPoint(MAIN_SERVER_IP_ADDRESS, MAIN_SERVER_PORT)) )
		{
			// Exception
			std::cerr << "FATAL: Failed to bind listener.\n";
			result = false;
		}
		else if ( const BOOL on = true;
				 -1 == ::setsockopt(listener.handle(), SOL_SOCKET, SO_CONDITIONAL_ACCEPT, (char*)&on, sizeof(BOOL)) )
		{
			// Exception
			std::cerr << "FATAL: Failed to set socket option.\n";
			result = false;
		}
		else if ( -1 == iocp.add(listener.handle(), LISTENER_IDX) )
		{
			// Exception
			std::cerr << "FATAL: Failed to add listener into IOCP.\n";
			result = false;
		}
		else
		{
			listener.listen( BACKLOG_SIZ );
		}

		if ( false == result )
		{
			listener.close( );
			WSACleanup( );
			return -1;
		}
	}

	uint16_t numOfAcceptancePending = 0;
	std::array<Client, CLIENT_CAPACITY> clients;
	std::unordered_set<ClientIndex> clientIndicesNotAccepted;
	clientIndicesNotAccepted.reserve( CLIENT_CAPACITY );
	for ( uint16_t index = 0; index != CLIENT_CAPACITY; ++index )
	{
		Client& client = clients[index];
		client.setIndex( index );
		Socket& socket = client.socket();
		// NOTE: 디버그하기 쉬워서 주소 대신 인덱스를 키로 삼았습니다.
		if ( -1 == iocp.add(socket.handle(), index) )
		{
			std::cerr << "FATAL: Failed to add Client " << index << " into IOCP.\n";
			listener.close( );
			WSACleanup( );
			return -1;
		}

		if ( index < BACKLOG_SIZ )
		{
			const int result = listener.acceptOverlapped(socket, index);
			if ( FALSE == result )
			{
				// Exception
				std::cerr << "FATAL: Failed to accept.\n";
				listener.close( );
				WSACleanup( );
				return -1;
			}
			else if ( -1 == result )
			{
				// Exception
				std::cerr << "FATAL: Failed to get AcceptEx(...).\n";
				listener.close( );
				WSACleanup( );
				return -1;
			}
			++numOfAcceptancePending;
		}
		else
		{
			clientIndicesNotAccepted.emplace( index );
		}
	}

	uint16_t nOfConnInMainServ = 0;
	// NOTE: If T is less than about 64 Bytes,
	// std::vector<T> is as fast as std::forward_list<T> and much easier in erasing something in it.
	std::vector<ClientIndex> lobby;
	lobby.reserve( LOBBY_CAPACITY );
	std::unordered_map<HashedKey, Room> rooms;
	rooms.reserve( ROOM_CAPACITY );
	auto forceDisconnection =
		[&iocp, &clients, &clientIndicesNotAccepted, &nOfConnInMainServ, &lobby, &rooms]( const ClientIndex index ) -> bool
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
			if ( 0 == it->second.pop(index) )
			{
#ifdef _DEV
				std::cout << "Room "
					<< it->first << " has been destructed.\n";
#endif
				rooms.erase( it );
			}
		}
		// NOTE: Client 인스턴스를 통째로 새로 만들고 복사해 덮어쓰지 않고,
		// 필요한 부분만 교체해 작업을 최소화했습니다.
		client.reset( false );
		const Socket& socket = client.socket();
		if ( -1 == iocp.add(socket.handle(), index) )
		{
			// Exception
			std::cerr << "FATAL: Failed to add Client " << index << " into IOCP.\n";
			result = false;
			return result;
		}
		--nOfConnInMainServ;
		clientIndicesNotAccepted.emplace( index );
		std::cout << "Forced to disconnect Client " << index << ". (Now "
			<< nOfConnInMainServ << "/" << CLIENT_CAPACITY << " connections.)\n";
		return result;
	};
	ClientIndex queueServerIdx = UINT16_MAX;
	std::cout << "###############\n### MAIN SERVER\n###############\n\nQ. What would the queue server say?" << std::endl;
	std::string sign;
	std::cin >> sign;
	const HashedKey encryptedSign = ::util::hash::Digest2(sign);
	std::cout << "Ready.\n";
	std::vector<Ticket> tickets;
	std::unordered_set<ClientIndex> connectionsNotSubmittingTicket;
	IOCPEvent event;
	while ( true == IsWorking )
	{
#ifdef _DEV
		Clock::time_point begin = Clock::now();
#endif
		iocp.wait( event, 1 );
		for ( uint16_t i = 0; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[i];
////
// When event comes from listener,
////
			if ( LISTENER_IDX == (ClientIndex)ev.lpCompletionKey )
			{
				const ClientIndex candidateIdx = listener.completedIO(ev.lpOverlapped,
																	  ev.dwNumberOfBytesTransferred);
				Socket& candidateClientSocket = clients[candidateIdx].socket();
				if ( -1 == candidateClientSocket.updateAcceptContext(listener) )
				{
					// Exception
					std::cerr << "FATAL: Failed to update acception context of Candidate " <<
						candidateIdx << ".\n";
					// Break twice
					goto cleanUp;
				}
				// When accepting successfully,
				--numOfAcceptancePending;
				++nOfConnInMainServ;
				Socket& clientSocket = candidateClientSocket;
				const ClientIndex clientIdx = candidateIdx;
				if ( -1 == clientSocket.receiveOverlapped() )
				{
					// Exception
					std::cerr << "WARNING: Failed to receive from a new client " <<
						clientIdx << ".\n";
					if ( false == forceDisconnection(clientIdx) )
					{
						// Break twice
						goto cleanUp;
					}
					continue;
				}
#ifdef _DEV
				std::cout << "A new client " << clientIdx << " joined. (Now "
						<< nOfConnInMainServ << "/" << CLIENT_CAPACITY << " connections.)\n";
#endif
				clients[clientIdx].resetTimeStamp();
				connectionsNotSubmittingTicket.emplace( clientIdx );
			}
////
// When event comes from the queue server,
////
			else if ( queueServerIdx == (ClientIndex)ev.lpCompletionKey )
			{
				if ( auto it = clientIndicesNotAccepted.find(queueServerIdx);
					clientIndicesNotAccepted.end() != it )
				{
					std::cerr << "WARNING: Queue server has already disconnected.\n";
					continue;
				}
				Client& queueServer = clients[queueServerIdx];
				Socket& socketToQueueServer = queueServer.socket();
				const IOType cmpl = (IOType)socketToQueueServer.completedIO(ev.lpOverlapped,
																	ev.dwNumberOfBytesTransferred);
				if ( IOType::DISCONNECT == cmpl )
				{
					queueServer.reset( );
					--nOfConnInMainServ;
					clientIndicesNotAccepted.emplace( queueServerIdx );
					// Reset
					queueServerIdx = -1;
#ifdef _DEV
					std::cerr << "WARNING: The queue server disconnected. (Now "
						<< nOfConnInMainServ << '/' << CLIENT_CAPACITY << " connections.)\n";
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
							const char* const rcvBuf = socketToQueueServer.receivingBuffer();
							const std::string_view strView( rcvBuf );
							// When the queue server asked how many clients keep connecting,
							if ( const size_t pos = strView.find(TAG_NUM_OF_CONNECTIONS);
								std::string_view::npos != pos )
							{
#ifdef _DEV
								std::cout << "Queue server asks how many connections there are.\n";
#endif
								Packet packet;
								packet.pack( TAG_NUM_OF_CONNECTIONS, nOfConnInMainServ );
								if ( -1 == socketToQueueServer.sendOverlapped(packet) )
								{
									// Exception
									std::cerr << "WARNING: Failed to send the number of connections.\n";
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
#ifdef _DEV
									std::cout << "The number of connections has been told: "
										<< nOfConnInMainServ << ".\n";
#endif
								}
							}
							else
							{
								uint16_t off = 0;
								// When having received copies of the issued ticket,
								constexpr uint8_t TAG_TICKET_LEN = ::util::hash::Measure(TAG_TICKET);
								while ( true )
								{
									const size_t tagPos = strView.find(TAG_TICKET, off);
									if ( std::string_view::npos == tagPos )
									{
										break;
									}
 									const uint16_t beginPos = (uint16_t)tagPos + TAG_TICKET_LEN;
									off += sizeof(Ticket);
									const Ticket id = ::ntohl(*(Ticket*)&rcvBuf[beginPos]);
									tickets.emplace_back( id );
#ifdef _DEV
									std::cout << "A copy of the issued ticket arrived: " << id << ".\n";
#endif
								}
							}
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
					}
				}
			}
////
// When event comes from clients,
////
			else
			{
				const ClientIndex clientIdx = (ClientIndex)ev.lpCompletionKey;
				if ( auto it = clientIndicesNotAccepted.find(clientIdx);
					clientIndicesNotAccepted.end() != it )
				{
					std::cerr << "Client " << clientIdx << " has already disconnected.\n";
					continue;
				}
				Client& client = clients[clientIdx];
				Socket& clientSocket = client.socket();
				const IOType cmpl =	(IOType)clientSocket.completedIO(ev.lpOverlapped,
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
						if ( 0 == it->second.pop(clientIdx) )
						{
#ifdef _DEV
							std::cout << "Room "
								<< it->first << " has been destructed.\n";
#endif
							rooms.erase( it );
						}
					}
					client.reset( );
					--nOfConnInMainServ;
					clientIndicesNotAccepted.emplace( clientIdx );
#ifdef _DEV
					std::cout << "Client " << clientIdx << " left. (Now "
						<< nOfConnInMainServ << "/" << CLIENT_CAPACITY << " connections.)\n";
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
								connectionsNotSubmittingTicket.erase( clientIdx );
								tickets.erase( it );
//TODO: DB로부터 닉네임 가져오기.
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
							// When the queue server connects as a client,
							else if ( UINT16_MAX == queueServerIdx &&
									 encryptedSign == ::ntohl(*(HashedKey*)rcvBuf) )
							{
								queueServerIdx = clientIdx;
								connectionsNotSubmittingTicket.erase( queueServerIdx );
								client.setState( Client::State::AS_QUEUE_SERVER );
#ifdef _DEV
								std::cout << "Queue server asks how many connections there are.\n";
#endif
								Packet packet;
								packet.pack( TAG_NUM_OF_CONNECTIONS, nOfConnInMainServ );
								Socket& queueServerSocket = clientSocket;
								if ( -1 == queueServerSocket.sendOverlapped(packet) )
								{
									// Exception
									std::cerr << "WARNING: Failed to send the number of connections.\n";
									if ( false == forceDisconnection(queueServerIdx) )
									{
										// Break twice
										goto cleanUp;
									}
									continue;
								}
#ifdef _DEV
								std::cout << "The number of connections has been told: " << nOfConnInMainServ << ".\n";
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
								if ( UINT16_MAX == queueServerIdx )
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

		const Clock::time_point now = begin;
		for ( const ClientIndex idx : connectionsNotSubmittingTicket )
		{
			if ( TICKET_SUBMISSION_TIME_LIMIT < now - clients[idx].timeStamp() )
			{
				Socket& socket = clients[idx].socket();
				if ( FALSE == socket.disconnectOverlapped() )
				{
					// Exception
					std::cerr << "WARNING: Failed to disconnect Client " << idx << ".\n";
					if ( false == forceDisconnection(idx) )
					{
						// Exception
						// Break twice
						goto cleanUp;
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

		if ( numOfAcceptancePending < BACKLOG_SIZ )
		{
			for ( const ClientIndex idx : clientIndicesNotAccepted )
			{
				if ( TCP_TIMED_WAIT_DELAY_SEC < now - clients[idx].timeStamp() )
				{
					if ( FALSE == listener.acceptOverlapped(clients[idx].socket(),
															idx) )
					{
						// Exception
						std::cerr << "FATAL: Overlapped acceptEx failed.\n";
						goto cleanUp;
					}
					clientIndicesNotAccepted.erase( idx );
					++numOfAcceptancePending;
					break;
				}
			}
		}

#ifdef _DEV
		auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(Clock::now()-begin).count();
		if ( 10 < elapsed )
		{
			std::cout << "MS per frame: " << elapsed << "\n";
		}
#endif
	}

cleanUp:
	//!IMPORTANT: Must release all the overlapped I/O in hand before closing the server.
	//	 		  Otherwise overlapped I/O still runs on O/S background.
	std::cout << "Server gets closed.\n";
	listener.close( );
	uint16_t areClientsPending = 0;
	for ( Client& client : clients )
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
		for ( uint16_t i = 0; i != event.eventCount; ++i )
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
	std::cout << "Server has been closed successfully." << std::endl;

	WSACleanup( );

	return 0;
}