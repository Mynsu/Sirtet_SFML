#include "pch.h"
#include <Lib/Common.h>

using Ticket = HashedKey;
using ClientIndex = uint32_t;
const uint16_t CAPACITY = 100;
const uint16_t BACKLOG_SIZ = CAPACITY/2;
// Capacity in the main server defaults to 10000.
// You can resize it indirectly here without re-compliing or rescaling the main server.
// !IMPORTANT: This must be less than the real capacity of the server.
const uint16_t MAIN_SERVER_CAPACITY = 5;
const ClientIndex MAIN_SERVER_INDEX = CAPACITY;
const ClientIndex LISTENER_IDX = CAPACITY + 1;
const Clock::duration IDENTIFICATION_TIME_LIMIT = std::chrono::seconds(2);
const Clock::duration TCP_TIMED_WAIT_DELAY = std::chrono::seconds(30);
const Clock::duration NOTIFYING_QUEUE_INTERVAL = std::chrono::milliseconds(1000);
const Clock::duration ASKING_POP_INTERVAL = std::chrono::milliseconds(1000);

volatile bool IsWorking = true;
void ProcessSignal( int signal )
{
	if ( SIGINT == signal )
	{
		IsWorking = false;
	}
}
inline void PrintLeavingWithError( const uint32_t clientIndex, const uint32_t population )
{
	std::cout << "Client " << clientIndex << " left due to an error. (Now " <<
		population << '/' << CAPACITY << " connections.)\n";
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
			0 != ::setsockopt(socketToMainServer->handle(), SOL_SOCKET, SO_RCVBUF,
		(char*)&RCV_BUF_SIZ, sizeof(DWORD)) )
	{
		// Exception
		std::cerr << "FATAL: Failed to set receiving buffer size to zero for the main server.\n";
		result = false;
		return result;
	}
	if ( const DWORD SND_BUF_SIZ = 0;
		0 != ::setsockopt(socketToMainServer->handle(), SOL_SOCKET, SO_SNDBUF,
		(char*)&SND_BUF_SIZ, sizeof(DWORD)) )
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
	NONE_MAX,
};

class Client
{
public:
	Client( const Socket::Type type )
		: mSocket( type )
	{ }
	
	Clock::time_point timeStamp() const
	{
		return mTimeStamp;
	}
	void resetTimeStamp()
	{
		mTimeStamp = Clock::now();
	}
	Socket& socket()
	{
		return mSocket;
	}
private:
	Clock::time_point mTimeStamp;
	Socket mSocket;
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
	std::bitset<(int)Status::NONE_MAX> statusAgainstMainServer;
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
			std::cerr << "FATAL: Failed to add a listener into IOCP.\n";
			result = false;
		}
		else
		{
			listener.listen( BACKLOG_SIZ );
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

	uint32_t numOfAcceptancePending = 0;
	std::unordered_set<ClientIndex> clientIndicesNotAccepted;
	std::vector<Client> clients;
	clients.reserve( CAPACITY );
	for ( uint32_t i = 0; i != CAPACITY; ++i )
	{
		Client& client = clients.emplace_back(Socket::Type::TCP);
		Socket& socket = client.socket();
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

		if ( i < BACKLOG_SIZ )
		{
			const int result = listener.acceptOverlapped(socket, i);
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
			++numOfAcceptancePending;
		}
		else
		{
			clientIndicesNotAccepted.emplace( i );
		}
	}

	////
	//
	////

	uint32_t population = 0;
	auto forceDisconnection =
		[ &iocp, &clients, &clientIndicesNotAccepted, &population ]( const ClientIndex index ) -> bool
	{
		bool result = true;
		Socket& clientSocket = clients[index].socket();
		clientSocket.close( );
		clientSocket.reset( false );
		if ( -1 == iocp.add(clientSocket.handle(), index) )
		{
			// Exception
			result = false;
			return result;
		}
		--population;
		clientIndicesNotAccepted.emplace( index );
		return result;
	};
	uint32_t roomInMainServer = 0;
	uint32_t seedForInvitation = 0;
	HashedKey invitations[2];
	Clock::time_point lastTimeNotifyingQueue;
	Clock::time_point lastTimeAskingPop;
	std::vector<ClientIndex> queue;
	std::unordered_set<ClientIndex> connectionsUnidentified;
	std::bitset<CAPACITY> clientIndicesTicketed;
	IOCPEvent event;
	while ( true == IsWorking )
	{
		iocp.wait( event, 100 );
		for ( uint32_t i = 0; i != event.eventCount; ++i )
		{
			const OVERLAPPED_ENTRY& ev = event.events[i];
			////
			// When event comes from listener,
			////
			if ( LISTENER_IDX == (ClientIndex)ev.lpCompletionKey )
			{
				const ClientIndex candidateIdx = listener.completedIO(ev.lpOverlapped,
																	  ev.dwNumberOfBytesTransferred);
				Socket& candidateSocket = clients[candidateIdx].socket();
				if ( -1 == candidateSocket.updateAcceptContext(listener) )
				{
					// Exception
					std::cerr << "FATAL: Failed to update acception context of Client " << candidateIdx << ".\n";
					// NOTE: Break and break
					goto cleanUp;
				}
				// When accepting successfully,
				--numOfAcceptancePending;
				++population;
				Socket& clientSocket = candidateSocket;
				const ClientIndex clientIdx = candidateIdx;
#ifdef _DEBUG
				std::cout << "A new client " << clientIdx <<
					" wants to join the main server. (Now " <<
					population << "/" << CAPACITY << " connections.)\n";
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
					PrintLeavingWithError( clientIdx, population );
					continue;
				}
				clients[clientIdx].resetTimeStamp( );
				connectionsUnidentified.emplace( clientIdx );
			}
			////
			// When event comes from the main server,
			////
			else if ( MAIN_SERVER_INDEX == (ClientIndex)ev.lpCompletionKey )
			{
				const IOType cmpl = (IOType)socketToMainServer->completedIO(ev.lpOverlapped,
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
				if ( auto it = clientIndicesNotAccepted.find(clientIdx);
					clientIndicesNotAccepted.end() != it )
				{
					std::cerr << "WARNING: Client " << clientIdx << " has already disconnected.\n";
					continue;
				}
				Socket& clientSocket = clients[clientIdx].socket();
				const IOType cmpl = (IOType)clientSocket.completedIO(ev.lpOverlapped,
															 ev.dwNumberOfBytesTransferred);
				if ( IOType::DISCONNECT == cmpl )
				{
					clientSocket.reset( );
					--population;
					clients[clientIdx].resetTimeStamp();
					clientIndicesNotAccepted.emplace( clientIdx );
					if ( false == clientIndicesTicketed.test(clientIdx) )
					{
#ifdef _DEBUG
						std::cout << "A client " << clientIdx <<
							" left without a ticket. (Now "
							<< population << '/' << CAPACITY << " connections.)\n";
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
						std::cout << "A client " << clientIdx <<
							" left with a ticket. (Now "
							<< population << '/' << CAPACITY << " connections.)\n";
#endif
						clientIndicesTicketed.reset( clientIdx );
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
						PrintLeavingWithError( clientIdx, population );
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
							const WallClock::time_point wnow = WallClock::now();
							const std::chrono::seconds atSec =
								std::chrono::duration_cast<std::chrono::seconds>(wnow.time_since_epoch());
							const uint32_t seed = (uint32_t)atSec.count();
							if ( seed != seedForInvitation )
							{
								seedForInvitation = seed;
								std::minstd_rand engine( seed + SALT );
								engine.discard( engine() % MAX_KEY_STRETCHING );
								invitations[0] = engine();
								engine.seed( seed - 1 + SALT );
								engine.discard( engine() % MAX_KEY_STRETCHING );
								invitations[1] = engine();
							}
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
								connectionsUnidentified.erase( clientIdx );
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
									PrintLeavingWithError( clientIdx, population );
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
									std::cerr << "WARNING: Failed to disconnect Client " <<
										clientIdx << ".\n";
									if ( false == forceDisconnection(clientIdx) )
									{
										// Exception
										// Break twice
										goto cleanUp;
									}
									PrintLeavingWithError( clientIdx, population );
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
							if ( true == clientIndicesTicketed.test(clientIdx) )
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
									clientIndicesTicketed.reset( clientIdx );
									PrintLeavingWithError( clientIdx, population );
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

		const Clock::time_point now = Clock::now();
		for ( const ClientIndex idx : connectionsUnidentified )
		{
			if ( IDENTIFICATION_TIME_LIMIT < now - clients[idx].timeStamp() )
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
					PrintLeavingWithError( idx, population );
				}
			}
		}

		// When room in the main server remains yet,
		for ( auto it = queue.cbegin(); queue.cend() != it &&
										0 < roomInMainServer; )
		{
			Socket& clientSocket = clients[*it].socket();
			std::random_device d;
			std::minstd_rand engine( d() );
			const Ticket ticket = engine();
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
				PrintLeavingWithError( *it, population );
			}
			else
			{
#ifdef _DEBUG
				std::cout << "Sending a ticket " << ticket << " to Client " <<
					*it << " succeeded.\n";
#endif
				clientIndicesTicketed.set( *it );
				if ( -1 == socketToMainServer->sendOverlapped(packet) )
				{
					// Exception
					std::cerr << "WARNING: Failed to send a copy of the issued ticket to the main server.\n";
					if ( false == ResetAndReconnectToMainServer(socketToMainServer,
																iocp) )
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
		uint32_t order = 1;
		for ( auto it = queue.cbegin(); queue.cend() != it &&
										NOTIFYING_QUEUE_INTERVAL < now-lastTimeNotifyingQueue; )
		{
			// Reset
			lastTimeNotifyingQueue = now;

			Packet packet;
			packet.pack( TAG_ORDER_IN_QUEUE, order );
			++order;
			Socket& clientSocket = clients[*it].socket();
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
				PrintLeavingWithError( *it, population );
				it = queue.erase(it);
				continue;
			}
#ifdef _DEBUG
			std::cout << "Sending its order to the client " << *it <<
				" waiting in the queue line succeeded.\n";
#endif
			++it;
		}

		// When there's no more room in the main server,
		if ( 0 == roomInMainServer &&
			0 == statusAgainstMainServer[Status::WAITING_FOR_POPULATION] &&
			ASKING_POP_INTERVAL < now-lastTimeAskingPop )
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

		if ( numOfAcceptancePending < BACKLOG_SIZ )
		{
			for ( const ClientIndex idx : clientIndicesNotAccepted )
			{
				if ( TCP_TIMED_WAIT_DELAY < now-clients[idx].timeStamp() )
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
	}

cleanUp:
	//!IMPORTANT: Must release all the overlapped I/O in hand before closing the server.
	//	 		  Otherwise overlapped I/O still runs on O/S background.
	std::cout << "Server gets closed.\n";
	socketToMainServer->close( );
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
		   true == listener.isPending() ||
		   true == socketToMainServer->isPending() )
	{
		IOCPEvent event;
		iocp.wait( event, 100 );
		// 궁금: 소켓 닫으면 이벤트 발생?
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
				clients[idx].socket().completedIO( ev.lpOverlapped,
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