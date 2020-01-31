#include "pch.h"

using Ticket = HashedKey;
using ClientIndex = uint32_t;
const uint32_t CAPACITY = 100;
// Capacity in the main server defaults to 10000.
// You can resize it indirectly here without re-compliing or rescaling the main server.
// !IMPORTANT: This must be less than the real capacity of the server.
const uint32_t MAIN_SERVER_CAPACITY = 5;
const ClientIndex MAIN_SERVER_INDEX = CAPACITY;
const ClientIndex LISTENER_IDX = CAPACITY + 1;
const Clock::duration NOTIFYING_QUEUE_INTERVAL_MS = std::chrono::milliseconds(1000);
const Clock::duration ASKING_POP_INTERVAL_MS = std::chrono::milliseconds(1000);

volatile bool IsWorking = true;
void ProcessSignal( int signal )
{
	if ( SIGINT == signal )
	{
		IsWorking = false;
	}
}
inline void PrintLeavingWithError( const uint32_t clientIndex, const uint32_t numOfCandidates )
{
	std::cout << "Client " << clientIndex << " left due to an error. (Now " <<
		CAPACITY-numOfCandidates << '/' << CAPACITY << " connections.)\n";
}
// Returns 0 in success, -1 in fail.
inline bool ResetAndReconnectToMainServer( std::unique_ptr<Socket>& socketToMainServer,
										 IOCP& iocp )
{
	bool result = true;
	if ( nullptr != socketToMainServer )
	{
		socketToMainServer->close( );
		socketToMainServer.reset( );
	}
	socketToMainServer = std::make_unique<Socket>(Socket::Type::TCP);
	if ( -1 == socketToMainServer->bind(EndPoint::Any) )
	{
		// Exception
		std::cerr << "FATAL: Failed to bind a to-main-server socket.\n";
		result = false;
		return result;
	}
	if ( const DWORD RCV_BUF_SIZ = 0;
			0 != ::setsockopt(socketToMainServer->handle(), SOL_SOCKET, SO_RCVBUF, (char*)&RCV_BUF_SIZ, sizeof(DWORD)) )
	{
		// Exception
		std::cerr << "FATAL: Failed to set receiving buffer size to zero for the main server.\n";
		result = false;
		return result;
	}
	if ( const DWORD SND_BUF_SIZ = 0;
		0 != ::setsockopt(socketToMainServer->handle(), SOL_SOCKET, SO_SNDBUF, (char*)&SND_BUF_SIZ, sizeof(DWORD)) )
	{
		// Exception
		std::cerr << "FATAL: Failed to set sending buffer size to zero for the main server.\n";
		result = false;
		return result;
	}
	uint8_t tolerance = 0;
	while ( -1 == socketToMainServer->connect(EndPoint(MAIN_SERVER_IP_ADDRESS, MAIN_SERVER_PORT)) )
	{
		// Exception
		std::cerr << "WARNING: Failed to connect to the main server.\n";
		++tolerance;
		if ( 3 == tolerance )
		{
			result = false;
			return result;
		}
		std::this_thread::sleep_for( std::chrono::seconds(1) );
	}
	std::cout << "Succeeded in connecting to the main server.\n";
	if ( -1 == iocp.add(socketToMainServer->handle(), MAIN_SERVER_INDEX) )
	{
		// Exception
		std::cerr << "FATAL: Failed to add a to-main-server socket into IOCP.\n";
		result = false;
		return result;
	}
	std::cout << "\nWhat should I say to the main server to let him know I'm the queue server?" << std::endl;
	std::string sign;
	std::cin >> sign;
	const HashedKey encryptedSign = ::util::hash::Digest(sign.data(), (uint8_t)sign.size());
	Packet packet;
	packet.pack( "", encryptedSign );
	if ( -1 == socketToMainServer->sendOverlapped(packet) )
	{
		// Exception
		std::cerr << "WARNING: Failed to send sign to the main server.\n";
		result = false;
		return result;
	}
	if ( -1 == socketToMainServer->receiveOverlapped() )
	{
		// Exception
		std::cerr << "WARNING: Failed to receive from the main server.\n";
		result = false;
		return result;
	}
	std::cout << "Waiting for identification by the main server.\n";
	return result;
}

enum Status
{
	WAITING_FOR_IDENTIFICATION,
	WAITING_FOR_POPULATION,
};

int main( )
{
	signal( SIGINT, &ProcessSignal );
	
	{
		WSAData w;
		WSAStartup( MAKEWORD(2, 2), &w );
	}

	IOCP iocp( 2 );

	////
	//
	////

	std::cout << "\n###############\n### QUEUE SERVER\n###############\n";
	// This make it possible to replace the socket immediately, differently from DisconnectEx(...).
	std::unique_ptr<Socket> socketToMainServer;
	if ( false == ResetAndReconnectToMainServer(socketToMainServer, iocp) )
	{
		socketToMainServer->close( );
		WSACleanup( );
		return -1;
	}
	std::bitset< 8 > statusAgainstMainServer;
	statusAgainstMainServer.set( Status::WAITING_FOR_IDENTIFICATION, 1 );
	statusAgainstMainServer.set( Status::WAITING_FOR_POPULATION, 1 );

	////
	//
	////

	Socket listener( Socket::Type::TCP );
	{
		bool result = true;
		if ( -1 == listener.bind(EndPoint(QUEUE_SERVER_IP_ADDRESS, QUEUE_SERVER_PORT)) )
		{
			// Exception
			std::cerr << "FATAL: Failed to bind a listener.\n";
			result = false;
		}
		else if ( const DWORD RCV_BUF_SIZ = 0;
				 -1 == ::setsockopt(listener.handle(), SOL_SOCKET, SO_RCVBUF, (char*)&RCV_BUF_SIZ, sizeof(DWORD)) )
		{
			// Exception
			std::cerr << "FATAL: Failed to set receiving buffer size to zero for the listener.\n";
			result = false;
		}
		else if ( const DWORD SND_BUF_SIZ = 0;
				 -1 == ::setsockopt(listener.handle(), SOL_SOCKET, SO_SNDBUF, (char*)&SND_BUF_SIZ, sizeof(DWORD)) )
		{
			// Exception
			std::cerr << "FATAL: Failed to set sending buffer size to zero for the listener.\n";
			result = false;
		}
		else if ( -1 == iocp.add(listener.handle(), LISTENER_IDX) )
		{
			// Exception
			std::cerr << "FATAL: Failed to add a listener into IOCP.\n";
			result = false;
		}
		else
		{
			listener.listen( );
		}

		if ( false == result )
		{
			listener.close( );
			socketToMainServer->close( );
			IOCPEvent event;
			iocp.wait( event, INFINITE );
			WSACleanup( );
			return -1;
		}
	}

	////
	//
	////

	std::vector<Socket> clients;
	clients.reserve( CAPACITY );
	container::IteratoredQueue<ClientIndex> candidates;
	for ( uint32_t i = 0; i != CAPACITY; ++i )
	{
		Socket& socket = clients.emplace_back(Socket::Type::TCP);
		if ( -1 == iocp.add(socket.handle(), i) )
		{
			// Exception
			std::cerr << "FATAL: Failed to add a client into IOCP.\n";
			clients.clear( );
			listener.close( );
			socketToMainServer->close( );
			IOCPEvent event;
			iocp.wait( event, INFINITE );
			WSACleanup( );
			return -1;
		}
		candidates.emplace_back( i );
	}
	{
		const int result = listener.acceptOverlapped(clients.at(candidates.front()));
		if ( FALSE == result )
		{
			// Exception
			std::cerr << "FATAL: Failed to accept.\n";
			IsWorking = false;
		}
		else if ( -1 == result )
		{
			// Exception
			std::cerr << "FATAL: Getting AcceptEx failed.\n";
			IsWorking = false;
		}
	}

	////
	//
	////

	auto forceDisconnection = [ &clients, &iocp, &candidates ]( const ClientIndex index ) -> bool
	{
		bool result = true;
		Socket& clientSocket = clients[index];
		clientSocket.close( );
		clientSocket.reset( false );
		if ( -1 == iocp.add(clientSocket.handle(), index) )
		{
			// Exception
			result = false;
			return result;
		}
		candidates.emplace_back( index );
		return result;
	};
	bool wasBoatful = false;
	uint32_t roomInMainServer = 0;
	Clock::time_point lastTimeNotifyingQueue = Clock::now();
	Clock::time_point lastTimeAskingPop = Clock::now();
	std::list<ClientIndex> queue;
	IOCPEvent event;
	//궁금: std::unordered_set이 더 빠를까?
	std::bitset<CAPACITY> ticketedClients;
	while ( true == IsWorking )
	{
		iocp.wait( event, 100 );
		for ( uint32_t i = 0; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[ i ];
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
				Socket& candidateSocket = clients[candidate];
				//궁금: 동시에 accept 여럿하면 잘 되려나?
				if ( -1 == candidateSocket.updateAcceptContext(listener) )
				{
					// Exception
					std::cerr << "FATAL: Failed to update acception context of Candidate " << candidate << ".\n";
					// NOTE: Break and break
					goto cleanUp;
				}
				// When accepting successfully,
				candidates.pop_front( );
				Socket& clientSocket = candidateSocket;
				const ClientIndex clientIdx = candidate;
#ifdef _DEBUG
				std::cout << "A new client " << clientIdx <<
					" wants to join the main server. (Now " <<
					CAPACITY-candidates.size() << "/" << CAPACITY << " connections.)\n";
#endif
				if ( -1 == clientSocket.receiveOverlapped() )
				{
					// Exception
					std::cerr << "WARNING: Failed to receive from Client " <<
						clientIdx << ".\n";
					if ( false == forceDisconnection(clientIdx) )
					{
						// Exception
						// Break twice
						goto cleanUp;
					}
					PrintLeavingWithError( clientIdx, candidates.size() );
				}

				// Reloading the next candidate.
				// When room for the next client in THIS QUEUE SERVER, not the main server, remains yet,
				if ( 0 < candidates.size() )
				{
					const ClientIndex nextCandidate = candidates.front();
					if ( FALSE == listener.acceptOverlapped(clients[nextCandidate]) )
					{
						// Exception
						std::cerr << "FATAL: Overlapped acceptEx failed.\n";
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
			// When event comes from the main server,
			////
			else if ( MAIN_SERVER_INDEX == (ClientIndex)ev.lpCompletionKey )
			{
				const IOType cmpl = socketToMainServer->completedIO(ev.lpOverlapped,
																	ev.dwNumberOfBytesTransferred);
				if ( 0 == ev.dwNumberOfBytesTransferred )
				{
					if ( 1 == statusAgainstMainServer[Status::WAITING_FOR_IDENTIFICATION] )
					{
						std::cerr << "FATAL: Failed to get through identification by the main server.\n";
						IsWorking = false;
					}
					else
					{
						std::cerr << "WARNING: Main server closed or died.\n";
						if ( false == ResetAndReconnectToMainServer(socketToMainServer, iocp) )
						{
							// Exception
							// NOTE: Brake twice.
							goto cleanUp;
						}
						statusAgainstMainServer.set( Status::WAITING_FOR_IDENTIFICATION, 1 );
					}
				}
				else
				{
					switch ( cmpl )
					{
						case IOType::RECEIVE:
							if ( 1 == statusAgainstMainServer[Status::WAITING_FOR_IDENTIFICATION] )
							{
								std::cout << "Identification passed.\n";
								statusAgainstMainServer.reset( Status::WAITING_FOR_IDENTIFICATION );
							}
							// When having received population in the main server,
							if ( 1 == statusAgainstMainServer[Status::WAITING_FOR_POPULATION] )
							{
								statusAgainstMainServer.reset( Status::WAITING_FOR_POPULATION );
								const char* const rcvBuf = socketToMainServer->receivingBuffer();
								// NOTE: Assuming that there's a message about nothing but population in and from the main server.
								constexpr uint8_t TAG_POPULATION_LEN = ::util::hash::Measure(TAG_POPULATION);
								const uint32_t pop = ::ntohl(*(uint32_t*)&rcvBuf[TAG_POPULATION_LEN]);
								// When population is out of range,
								if ( pop<0 || MAIN_SERVER_CAPACITY<pop )
								{
									// Asking again the main server how many clients are there.
									Packet packet;
									const uint8_t ignored = 1;
									packet.pack( TAG_POPULATION, ignored );
									if ( -1 == socketToMainServer->sendOverlapped(packet) )
									{
										std::cerr << "WARNING: Failed to ask the main server how many clients are there.\n";
										if ( false == ResetAndReconnectToMainServer(socketToMainServer, iocp) )
										{
											goto cleanUp;
										}
										statusAgainstMainServer.set( Status::WAITING_FOR_IDENTIFICATION, 1 );
									}
									statusAgainstMainServer.set( Status::WAITING_FOR_POPULATION, 1 );
#ifdef _DEBUG
									std::cout << "Asked again the main server how many clients are there.\n";
#endif
								}
								else
								{
									roomInMainServer = (int)MAIN_SERVER_CAPACITY-pop;
#ifdef _DEBUG
									std::cout << "Room in the main server: " << roomInMainServer << std::endl;
#endif
								}
								//TODO: 받기 항상 걸어두고 싶다.
								if ( -1 == socketToMainServer->receiveOverlapped() )
								{
									std::cerr << "WARNING: Failed to receive population in the main server.\n";
									if ( false == ResetAndReconnectToMainServer(socketToMainServer, iocp) )
									{
										goto cleanUp;
									}
									statusAgainstMainServer.set( Status::WAITING_FOR_IDENTIFICATION, 1 );
								}
							}
							break;
						case IOType::SEND:
							break;
						default:
#ifdef _DEBUG
							__debugbreak( );
#else
							__assume( 0 );
#endif
							break;
					}
				}
			}
			////
			// When event comes from a client
			////
			else
			{
				const ClientIndex clientIdx = (ClientIndex)ev.lpCompletionKey;
				Socket& clientSocket = clients[clientIdx];
				const IOType cmpl = clientSocket.completedIO(ev.lpOverlapped,
															 ev.dwNumberOfBytesTransferred);
				if ( IOType::DISCONNECT == cmpl )
				{
					const bool isAlreadyCandidate = candidates.contains(clientIdx);
					if ( true == isAlreadyCandidate )
					{
						std::cerr << "WARNING: Client " << clientIdx << " is already candidate.\n";
					}
					else
					{
						clientSocket.reset( );
						candidates.emplace_back( clientIdx );
						if ( false == ticketedClients.test(clientIdx) )
						{
#ifdef _DEBUG
							const uint32_t numOfCandidates = candidates.size();
							std::cout << "A client " << clientIdx <<
								" left without a ticket. (Now "
								<< CAPACITY-numOfCandidates << '/' << CAPACITY << " connections.)\n";
#endif
							for ( auto it = queue.cbegin(); queue.cend() != it; ++it )
							{
								if ( clientIdx == *it )
								{
									queue.erase( it );
									break;
								}
							}
						}
						else
						{
#ifdef _DEBUG
							const uint32_t numOfCandidates = candidates.size();
							std::cout << "A client " << clientIdx <<
								" left with a ticket. (Now "
								<< CAPACITY-numOfCandidates << '/' << CAPACITY << " connections.)\n";
#endif
							ticketedClients.reset( clientIdx );
						}
					}
				}
				else if ( 0 == ev.dwNumberOfBytesTransferred )
				{
					const int result = clientSocket.disconnectOverlapped();
					if ( FALSE == result )
					{
						// Exception
						std::cerr << "WARNING: Failed to disconnect Client " << clientIdx << ".\n";
						if ( false == forceDisconnection(clientIdx) )
						{
							// Exception
							// Break twice
							goto cleanUp;
						}
						PrintLeavingWithError( clientIdx, candidates.size() );
						continue;
					}
					else if ( -1 == result )
					{
						std::cerr << "FATAL: Failed to get DisconnectEx(...) for the queue server, Client " <<
							clientIdx << ".\n";
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
							using WallClock = std::chrono::system_clock;
							const WallClock::time_point now = WallClock::now();
							const WallClock::duration tillNow = now.time_since_epoch();
							const std::chrono::minutes atMin = std::chrono::duration_cast<std::chrono::minutes>(tillNow);
							const HashedKey invitations[3] = { ::util::hash::Digest(VERSION + atMin.count() + SALT),
								::util::hash::Digest(VERSION + (atMin-std::chrono::minutes(1)).count() + SALT),
								::util::hash::Digest(VERSION + (atMin+std::chrono::minutes(1)).count() + SALT) };
							const char* const rcvBuf = clientSocket.receivingBuffer();
							constexpr uint8_t TAG_INVITATION_LEN = ::util::hash::Measure(TAG_INVITATION);
							const HashedKey submittedInvitation = ::ntohl(*(HashedKey*)&rcvBuf[TAG_INVITATION_LEN]);
							bool isIdentified = false;
							for ( const HashedKey inv : invitations	)
							{
								if ( inv == submittedInvitation )
								{
									isIdentified = true;
									break;
								}
							}
							if ( true == isIdentified )
							{
								if ( -1 == clientSocket.receiveOverlapped() )
								{
									// Exception
									std::cerr << "WARNING: Failed to receive from Client " <<
										clientIdx << ".\n";
									if ( false == forceDisconnection(clientIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
									PrintLeavingWithError( clientIdx, candidates.size() );
									continue;
								}
								queue.emplace_back( clientIdx );
							}
							else
							{
								std::cerr << "WARNING: Client " << clientIdx <<
									" submitted an invalid invitation.\n";
								const int result = clientSocket.disconnectOverlapped();
								if ( FALSE == result )
								{
									// Exception
									std::cerr << "WARNING: Failed to disconnect Client " << clientIdx << ".\n";
									if ( false == forceDisconnection(clientIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
									PrintLeavingWithError( clientIdx, candidates.size() );
								}
								else if ( -1 == result )
								{
									std::cerr << "FATAL: Failed to get DisconnectEx(...) for the queue server, Client " <<
										clientIdx << ".\n";
									// Break twice
									goto cleanUp;
								}
							}
							break;
						}
						case IOType::SEND:
							if ( true == ticketedClients.test(clientIdx) )
							{
								const int result = clientSocket.disconnectOverlapped();
								if ( FALSE == result )
								{
									// Exception
									std::cerr << "WARNING: Failed to disconnect Client " << clientIdx << ".\n";
									if ( false == forceDisconnection(clientIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
									ticketedClients.reset( clientIdx );
									PrintLeavingWithError( clientIdx, candidates.size() );
									continue;
								}
								else if ( -1 == result )
								{
									std::cerr << "FATAL: Failed to get DisconnectEx(...) for the queue server, Client " <<
										clientIdx << ".\n";
									// Break twice
									goto cleanUp;
								}
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
		}

		// When room in the main server remains yet,
		for ( auto it = queue.cbegin(); queue.cend()!=it && 0<roomInMainServer; )
		{
			Socket& clientSocket = clients[*it];
			const uint32_t salt = *it;
			const Ticket ticket = ::util::hash::Digest((uint32_t)(clientSocket.handle()+salt));
			Packet packet;
			packet.pack( TAG_TICKET, ticket );
			// First sent to a client, last to the main server.
			// Otherwise it's annoying to get rid of copies of the ticket in the main server
			// actually not issued to the client.
			if ( -1 == clientSocket.sendOverlapped(packet) )
			{
				// Exception
				std::cerr << "WARNING: Failed to send a ticket to Client " << *it
					<< " waiting in the queue line.\n";
				if ( false == forceDisconnection(*it) )
				{
					// Exception
					// Break twice
					goto cleanUp;
				}
				PrintLeavingWithError( *it, candidates.size() );
			}
			else
			{
#ifdef _DEBUG
				std::cout << "Sending a ticket " << ticket << " to Client " << *it << " succeeded.\n";
#endif
				ticketedClients.set( *it );
				if ( -1 == socketToMainServer->sendOverlapped(packet) )
				{
					// Exception
					std::cerr << "WARNING: Failed to send a copy of the issued ticket to the main server.\n";
					if ( false == ResetAndReconnectToMainServer(socketToMainServer, iocp) )
					{
						// Exception
						// Break twice.
						goto cleanUp;
					}
					statusAgainstMainServer.set( Status::WAITING_FOR_IDENTIFICATION, 1 );
				}
				--roomInMainServer;
			}
			it = queue.erase(it);
		}

		// Notifying their own order in the queue line.
		const Clock::time_point now = Clock::now();
		uint32_t order = 1;
		for ( auto it = queue.cbegin(); queue.cend()!=it && NOTIFYING_QUEUE_INTERVAL_MS<now-lastTimeNotifyingQueue; )
		{
			// Reset
			lastTimeNotifyingQueue = now;

			Packet packet;
			packet.pack( TAG_ORDER_IN_QUEUE, order );
			++order;
			Socket& clientSocket = clients[*it];
			if ( -1 == clientSocket.sendOverlapped(packet) )
			{
				// Exception
				std::cerr << "WARNING: Failed to send its order to Client " << *it
					<< " waiting in the queue line.\n";
				if ( false == forceDisconnection(*it) )
				{
					// Exception
					// Break twice
					goto cleanUp;
				}
				PrintLeavingWithError( *it, candidates.size() );
				it = queue.erase(it);
				continue;
			}
#ifdef _DEBUG
			std::cout << "Sending its order to the client " << *it << " waiting in the queue line succeeded.\n";
#endif
			++it;
		}

		// When there's no more room in the main server,
		if ( 0 == roomInMainServer &&
			0 == statusAgainstMainServer[Status::WAITING_FOR_POPULATION] &&
			ASKING_POP_INTERVAL_MS < now-lastTimeAskingPop )
		{
			// Reset
			lastTimeAskingPop = now;

			const uint8_t ignored = 1;
			Packet packet;
			packet.pack( TAG_POPULATION, ignored );
			if ( -1 == socketToMainServer->sendOverlapped(packet) )
			{
				// Exception
				std::cerr << "WARNING: Failed to ask the main server how many clients are there.\n";
				if ( false == ResetAndReconnectToMainServer(socketToMainServer, iocp) )
				{
					// Exception
					// Break twice.
					goto cleanUp;
				}
				statusAgainstMainServer.set( Status::WAITING_FOR_IDENTIFICATION, 1 );
			}
			statusAgainstMainServer.set( Status::WAITING_FOR_POPULATION, 1 );
#ifdef _DEBUG
			std::cout << "Asked the main server how many clients are there.\n";
#endif
		}

		if ( true == wasBoatful && 0 < candidates.size() )
		{
			wasBoatful = false;
			const ClientIndex nextCandidateClientIdx = candidates.front();
			if ( FALSE == listener.acceptOverlapped(clients[nextCandidateClientIdx]) )
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
	socketToMainServer->close( );
	listener.close( );
	uint32_t areClientsPending = 0;
	for ( auto& it : clients )
	{
		it.close( );
		if ( true == it.isPending() )
		{
			++areClientsPending;
		}
	}
	while ( 0 < areClientsPending ||
		   true == listener.isPending() ||
		   true == socketToMainServer->isPending() )
	{
		IOCPEvent event;
		iocp.wait( event, 100 );
		for ( uint32_t i = 0; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[i];
			const ClientIndex idx = (ClientIndex)ev.lpCompletionKey;
			if ( MAIN_SERVER_INDEX == idx )
			{
				socketToMainServer->completedIO( ev.lpOverlapped,
												ev.dwNumberOfBytesTransferred );
			}
			else if ( LISTENER_IDX == idx )
			{
				listener.completedIO( ev.lpOverlapped,
									 ev.dwNumberOfBytesTransferred );
			}
			else
			{
				clients[idx].completedIO( ev.lpOverlapped,
										 ev.dwNumberOfBytesTransferred );
				--areClientsPending;
			}
		}
	}
	clients.clear( );
	
	WSACleanup( );
	std::cout << "Server has been closed successfully." << std::endl;

	return 0;
}