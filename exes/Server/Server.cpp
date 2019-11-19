#include "pch.h"
#include "Room.h"
#include "Client.h"
#include "Tetrimino.h"

//TODO: 소켓 핸들 랜덤이 아닌데?, 접속 종료한 지 얼마 안 되었을 때 바로 접속 가능하게

//DOING: 로직

const uint32_t CAPACITY = 4u;
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
	const Index LISTENER_IDX = CAPACITY;
	if ( -1 == iocp.add(listener, LISTENER_IDX) )
	{
		// Exception
		std::cerr << "FATAL: Failed to add listener into IOCP.\n";
		listener.close( );
		WSACleanup( );
		return -1;
	}
	std::vector< Client > clientS;
	clientS.reserve( CAPACITY );
	// Must use push_back(or emplace_back), front and pop_front only, like std::queue.
	container::IteratoredQueue< Index > candidateS;
	candidateS.reserve( CAPACITY );
	for ( uint32_t i = 0; i != CAPACITY; ++i )
	{
		clientS.emplace_back( Socket::Type::TCP );
		if ( -1 == iocp.add(clientS.at(i).socket(), i) )
		{
			std::cerr << "FATAL: Failed to add Client " << i << " into IOCP.\n";
			listener.close( );
			WSACleanup( );
			clientS.clear( );
			return -1;
		}
		candidateS.emplace_back( i );
	}
	const int result = listener.acceptOverlapped( clientS.at(candidateS.front()).socket() );
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
	auto forceDisconnection = [ &clientS, &iocp, &candidateS, &roomS ]( const Index idx, const char* msg )->int
	{
		std::cerr << "WARNING: " << msg << " Client " << idx << ".\n";
		Client& client = clientS[ idx ];
		const RoomID roomID = client.roomID( );
		if ( 0 <= roomID )
		{
			roomS[ roomID ].mParticipantS.erase( idx );
			client.setRoomID( -1 );
		}
		client.setSocket( Socket(Socket::Type::TCP, Socket::CompletedWork::DISCONNECT) );
		candidateS.emplace_back( idx );
		std::cout << "Forced to disconnect Client " << idx << ". (Now "
			<< CAPACITY-candidateS.size() << "/" << CAPACITY << " connections.)\n";
		if ( -1 == iocp.add(client.socket(), idx) )
		{
			// Exception
			std::cerr << "FATAL: Failed to add Client " << idx << " into IOCP.\n";
			return -1;
		}
		return 0;
	};

	std::cout << "##########\n### MAIN SERVER\n##########\n\nWhat is today's salt, Sir?" << std::endl;
	std::string todaysSalt;
	std::cin >> todaysSalt;
	std::cout << "Ready.\n";
	const HashedKey refinedSalt = ::util::hash::Digest( todaysSalt.data(), (uint8_t)todaysSalt.size() );
	const std::string encryptedSalt( std::to_string(refinedSalt) );
	bool wasBoatful = false;
	Index queueServerIdx = -1;
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
			if ( LISTENER_IDX == (Index)ev.lpCompletionKey && 0 < candidateS.size() )
			{
				listener.pend( false );
				const Index candidateClientIdx = candidateS.front( );
				Socket& candidateClientSocket = clientS[ candidateClientIdx ].socket( );
				if ( -1 == candidateClientSocket.updateAcceptContext(listener) )
				{
					// Exception
					// Break twice
					goto cleanUp;
				}
				// When acception is successful,
				Socket& clientSocket = candidateClientSocket;
				const Index clientIdx = candidateClientIdx;
				if ( -1 == clientSocket.receiveOverlapped() )
				{
					// Exception
					if ( -1 == forceDisconnection(clientIdx, "Failed to pend reception from a new") )
					{
						// Break twice
						goto cleanUp;
					}
				}
				else
				{
					candidateS.pop_front( );
					clientSocket.pend( );
#ifdef _DEBUG
					std::cout << "A new client " << clientIdx << " joined. (Now "
						<< CAPACITY-candidateS.size() << "/" << CAPACITY << " connections.)\n";
#endif
				}

				// Reloading the next candidate.
				if ( 0 < candidateS.size() )
				{
					const Index nextCandidateIdx = candidateS.front( );
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
			else if ( queueServerIdx == (Index)ev.lpCompletionKey )
			{
				Socket& socketToQueueServer = clientS[ queueServerIdx ].socket( );
				socketToQueueServer.pend( false );
				if ( Socket::CompletedWork::DISCONNECT == socketToQueueServer.completedWork() )
				{
					const int res = candidateS.contains( queueServerIdx );
					if ( -1 == res )
					{
						std::cerr << "WARNING: Client " << queueServerIdx << " is already candidate.\n";
					}
					else if ( 0 == res )
					{
						candidateS.emplace_back( queueServerIdx );
						// Reset
						queueServerIdx = -1;
#ifdef _DEBUG
						std::cerr << "WARNING: The queue server disconnected. (Now "
							<< CAPACITY-candidateS.size( ) << '/' << CAPACITY << " connections.)\n";
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
						if ( -1 == forceDisconnection(queueServerIdx, "Forced to disconnect the queue server,") )
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
					switch ( socketToQueueServer.completedWork() )
					{
						case Socket::CompletedWork::RECEIVE:
						{
// TODO: getline
							const std::string_view strView( socketToQueueServer.receivingBuffer() );
							// When the queue server asked how many clients keep connecting,
							if ( std::string_view::npos != strView.find(encryptedSalt) )
							{
#ifdef _DEBUG
								std::cout << "Population has been asked by the queue server.\n";
#endif
								std::string pop( std::to_string(CAPACITY-candidateS.size()) );
								if ( -1 == socketToQueueServer.sendOverlapped(pop.data(),pop.size()) )
								{
									// Exception
									if ( -1 == forceDisconnection(queueServerIdx,
																   "Failed to pend sending population to the queue server,") )
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
									std::cout << "Population has been told: " << pop.data( ) << std::endl;
#endif
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
 								const uint32_t beginPos = static_cast< uint32_t >( tagPos ) + TAG_TICKET_LEN;
								off = strView.find( TOKEN_SEPARATOR, tagPos );
#ifdef _DEBUG
								if ( std::string_view::npos == off ) __debugbreak( );
#endif
								const uint32_t endPos = static_cast< uint32_t >( off );
								const std::string ticketID( strView.substr(beginPos, endPos-beginPos) );
								ticketS.emplace( static_cast<Ticket>(std::atoll(ticketID.data())) );
#ifdef _DEBUG
								std::cout << "A copy of the issued ticket arrived: " << ticketID << std::endl;
#endif
							}
							break;
						}
						case Socket::CompletedWork::SEND:
						{
// NOTE: Here is BLACK SPOT.
// Frequently-happening error: 10053, 10054.
							if ( -1 == socketToQueueServer.receiveOverlapped() )
							{
								// Exception
								if ( -1 == forceDisconnection(queueServerIdx,
															   "Failed to pend reception from the queue server,") )
								{
									// Break twice
									goto cleanUp;
								}
								// Reset
								queueServerIdx = -1;
								break;
							}
							socketToQueueServer.pend( );
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
				const Index clientIdx = (Index)ev.lpCompletionKey;
				Client& client = clientS[ clientIdx ];
				Socket& clientSocket = client.socket( );
				clientSocket.pend( false );
				const Socket::CompletedWork cmpl = clientSocket.completedWork( );
				if ( Socket::CompletedWork::DISCONNECT == cmpl )
				{
					const int res = candidateS.contains( clientIdx );
					if ( -1 == res )
					{
						std::cerr << "WARNING: Client " << clientIdx << " is already candidate.\n";
					}
					else if ( 0 == res )
					{
						candidateS.emplace_back( clientIdx );
#ifdef _DEBUG
						std::cout << "Client " << clientIdx << " left. (Now "
							<< CAPACITY-candidateS.size( ) << "/" << CAPACITY << " connections.)\n";
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

					const RoomID roomID = client.roomID( );
					if ( 0 <= roomID )
					{
						roomS[ roomID ].mParticipantS.erase( clientIdx );
						client.setRoomID( -1 );
					}
					const int res = clientSocket.disconnectOverlapped( );
					if ( 0 == res && ERROR_IO_PENDING != WSAGetLastError() )
					{
						// Exception
						if ( -1 == forceDisconnection(clientIdx, "Failed to disconnect") )
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
					switch( client.state() )
					{
						case Client::State::UNVERIFIED:
							switch ( cmpl )
							{
								case Socket::CompletedWork::RECEIVE:
								{
									const char* const rcvBuf = clientSocket.receivingBuffer( );
									// When having a copy of the ticket received from this client,
									if ( const Ticket ticket = (Ticket)std::atoll(&rcvBuf[TAG_TICKET_LEN]);
										ticketS.cend() != ticketS.find(ticket) )
									{
										client.holdTicket( ticket );
										ticketS.erase( ticket );
//TODO: 닉네임
										std::string nickname( TAG_NICKNAME );
										nickname += "nickname01";
										if ( -1 == clientSocket.sendOverlapped(nickname.data(), nickname.size()) )
										{
											// Exception
											if ( -1 == forceDisconnection(clientIdx, "Failed to acknowledge") )
											{
												// Break twice
												goto cleanUp;
											}
											break;
										}
										clientSocket.pend( );
									}
									// When the queue server as a client asked how many clients keep connecting,
									else if ( refinedSalt == (HashedKey)std::atoll(rcvBuf) )
									{
										queueServerIdx = clientIdx;
#ifdef _DEBUG
										std::cout << "Population has been asked by the queue server.\n";
#endif
										// Letting the queue server know the population here
										// makes the queue server possible
										// to scale up/down the virtual capacity of the main server elastically and independently.
										std::string pop( std::to_string(CAPACITY-candidateS.size()) );
										Socket& queueServerSocket = clientSocket;
										if ( -1 == queueServerSocket.sendOverlapped(pop.data(), pop.size()) )
										{
											// Exception
											if ( -1 == forceDisconnection(queueServerIdx, "Failed to pend sending population to") )
											{
												// Break twice
												goto cleanUp;
											}
											break;
										}
										queueServerSocket.pend( );
#ifdef _DEBUG
										std::cout << "Population has been told: " << pop.data( ) << std::endl;
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
											if ( -1 == forceDisconnection(clientIdx, "Failed to disconnect" ) )
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
									break;
								}
								case Socket::CompletedWork::SEND:
									client.setState( Client::State::IN_LOBBY );
									if ( -1 == clientSocket.receiveOverlapped() )
									{
										// Exception
										if ( -1 == forceDisconnection(clientIdx, "Failed to pend reception from" ) )
										{
											// Break twice
											goto cleanUp;
										}
										break;
									}
									clientSocket.pend( );
									break;
								default:
#ifdef _DEBUG
									__debugbreak( );
#else
									__assume(0);
#endif
							}
							break;
						case Client::State::IN_LOBBY:
							switch ( cmpl )
							{
								case Socket::CompletedWork::RECEIVE:
								{
									const char* const rcvBuf = clientSocket.receivingBuffer( );
									const Request req = (Request)std::atoi(rcvBuf);
									switch ( req )
									{
										case Request::CREATE_ROOM:
										{
											std::random_device rd;
											std::minstd_rand re( rd() );
											const RoomID roomID = (RoomID)re( );
											client.setRoomID( roomID );
											roomS.emplace( roomID, clientIdx );
											std::string response( TAG_CREATE_ROOM );
											if ( -1 == clientSocket.sendOverlapped(response.data(), response.size()) )
											{
												// Exception
												if ( -1 == forceDisconnection(clientIdx, "Failed to pend sending the room ID to") )
												{
													// Break twice
													goto cleanUp;
												}
												break;
											}
											clientSocket.pend( );
											client.setLatestRequest( req );
											break;
										}
										default:
										{
											std::cerr << "WARNING: Client " << clientIdx << " sent an undefined request.\n";
											if ( -1 == clientSocket.receiveOverlapped() )
											{
												// Exception
												if ( -1 == forceDisconnection(clientIdx, "Failed to pend reception from") )
												{
													// Break twice
													goto cleanUp;
												}
												break;
											}
											clientSocket.pend( );
											break;
										}
									}
									break;
								}
								case Socket::CompletedWork::SEND:
									switch( client.latestRequest() )
									{
										case Request::CREATE_ROOM:
											client.setState( Client::State::IN_ROOM );
											if ( -1 == clientSocket.receiveOverlapped() )
											{
												// Exception
												if ( -1 == forceDisconnection(clientIdx, "Failed to pend reception from") )
												{
													// Break twice
													goto cleanUp;
												}
												break;
											}
											clientSocket.pend( );
											break;
										default:
											break;
									}
									break;
								default:
#ifdef _DEBUG
									__debugbreak( );
#else
									__assume( 0 );
#endif
							}
							break;
						case ::Client::State::IN_ROOM:
							switch ( cmpl )
							{
								case ::Socket::CompletedWork::RECEIVE:
								{
									const char* const rcvBuf = clientSocket.receivingBuffer( );
									const Request req = (Request)std::atoi(rcvBuf);
									switch ( req )
									{
										case Request::START_GAME:
											if ( Room& room = roomS[client.roomID()];
												 room.hostIndex() == clientIdx )
											{
#ifdef _DEBUG
												std::cout << "Client " << clientIdx << " requested to create a room.\n";
#endif
												for ( const auto idx : room.mParticipantS )
												{
													Client& participant = clientS[ idx ];
													Socket& participantSocket = participant.socket( );
													std::string req( TAG_GET_READY );
													if ( -1 == participantSocket.sendOverlapped(req.data(), req.size()) )
													{
														// Exception
														if ( -1 == forceDisconnection( idx,
																					   "Failed to pend sending request to get ready to" ) )
														{
															// Break twice
															goto cleanUp;
														}
														break;
													}
													participantSocket.pend( );
													room.restartTimer( );
													clientS[ idx ].setState( Client::State::READY );
												}
												room.setState( Room::State::READY );
											}
											else
											{
												const int res = clientSocket.disconnectOverlapped( );
												if ( 0==res && ERROR_IO_PENDING!=WSAGetLastError( ) )
												{
													// Exception
													if ( -1 == forceDisconnection( clientIdx, "Failed to disconnect" ) )
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
											break;
//										case Request::INVITE:
//										{
//											const Index guestIdx = (Index)std::atoi(&rcvBuf[TAG_INVITE_LEN]);
////TODO	
//											if ( 0 != candidateS.contains(guestIdx) )
//											{
//												char response = RESPONSE_NEGATION;
//												if ( -1 == clientSocket.sendOverlapped(&response, 1) )
//												{
//													// Exception
//													if ( -1 == forceDisconnection(clientIdx, "Failed to tell invitation failure to") )
//													{
//														// Break twice
//														goto cleanUp;
//													}
//												}
//												else
//												{
//													clientSocket.pend( );
//												}
//												break;
//											}
//									
//											Client& guest = clientS[ guestIdx ];
//											guest.setState( Client::State::INVITED );
//											guest.setRoomID( !client.roomID() );
//											Socket& guestSocket = guest.socket( );
//											std::string _hostIdx( TAG_INVITE+std::to_string(clientIdx) );
//											if ( -1 == guestSocket.sendOverlapped(_hostIdx.data(), _hostIdx.size()) )
//											{
//												// Exception
//												if ( -1 == forceDisconnection(guestIdx, "Failed to invite") )
//												{
//													// Break twice
//													goto cleanUp;
//												}
//												break;
//											}
//											guestSocket.pend( );
//											char response = RESPONSE_AFFIRMATION;
//											if ( -1 == clientSocket.sendOverlapped(&response, 1) )
//											{
//												// Exception
//												if ( -1 == forceDisconnection( clientIdx, "Failed to tell invitation success to" ) )
//												{
//													// Break twice
//													goto cleanUp;
//												}
//												break;
//											}
//											clientSocket.pend( );
//											break;
//										}
										default:
										{
											std::cerr << "WARNING: Tried to disconnect Client " << clientIdx << " with the undefined behavior.\n";
											const int res = clientSocket.disconnectOverlapped( );
											if ( 0 == res && ERROR_IO_PENDING != WSAGetLastError( ) )
											{
												// Exception
												if ( -1 == forceDisconnection( clientIdx, "Failed to disconnect" ) )
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
											break;
										}
									}
									break;
								}
								case ::Socket::CompletedWork::SEND:
									break;
								default:
#ifdef _DEBUG
									__debugbreak( );
#else
									__assume(0);
#endif
							}
							break;
						case Client::State::READY:
							break;
						case Client::State::PLAYING:
							switch ( cmpl )
							{
								case Socket::CompletedWork::SEND:
									if ( -1 == clientSocket.receiveOverlapped() )
									{
										// Exception
										if ( -1 == forceDisconnection(clientIdx, "Failed to pend reception from") )
										{
											// Break twice
											goto cleanUp;
										}
										break;
									}
									clientSocket.pend( );
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
							break;
//						case Client::State::INVITED:
//							switch ( cmpl )
//							{
//								case Socket::CompletedWork::RECEIVE:
//								{
//									const char* const rcvBuf = clientSocket.receivingBuffer( );
//									if ( RESPONSE_AFFIRMATION == rcvBuf[0] )
//									{
//										const RoomID roomID = !client.roomID( );
//										std::string noti( TAG_NOTIFY_JOINING + clientIdx );
////TODO
//										std::string olders( TAG_OLDERS + TOKEN_SEPARATOR_2 );
//										for ( auto idx : roomS[roomID].mParticipantS )
//										{
//											Socket& participantSocket = clientS[ idx ].socket( );
////궁금: full-duplex 될까?  안 됐었는데.
//											if ( -1 == participantSocket.sendOverlapped(noti.data(), noti.size()) )
//											{
//												// Exception
//												if ( -1 == forceDisconnection( clientIdx, "Failed to notify joining to" ) )
//												{
//													// Break twice
//													goto cleanUp;
//												}
////TODO: 퇴장 알리기												
//												break;
//											}
//											participantSocket.pend( );
////TODO											
//											olders += std::to_string( idx ) + TOKEN_SEPARATOR_2;
//										}
//										if ( -1 == clientSocket.sendOverlapped(olders.data(), olders.size()) )
//										{
//											// Exception
//											if ( -1 == forceDisconnection( clientIdx, "Failed to notify joining to" ) )
//											{
//												// Break twice
//												goto cleanUp;
//											}
////TODO: 퇴장 알리기											
//											break;
//										}
//										roomS[ roomID ].join( clientIdx );
//										client.setState( Client::State::IN_ROOM );
//										clientSocket.pend( );
//									}
//									else
//									{
//										client.setRoomID( -1 );
//									}
//									break;
//								}
//								case Socket::CompletedWork::SEND:
//									if ( -1 == clientSocket.receiveOverlapped() )
//									{
//										// Exception
//										if ( -1 == forceDisconnection(clientIdx, "Failed to pend reception from") )
//										{
//											// Break twice
//											goto cleanUp;
//										}
//										break;
//									}
//									clientSocket.pend( );
//									break;
//								default:
//#ifdef _DEBUG
//									__debugbreak( );
//#else
//									__assume(0);
//#endif
//							}
//							break;
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
		}

		for ( auto it = roomS.begin(); roomS.end() != it; ++it )
		{
			Room& room = it->second;
			if ( Room::State::READY == room.state() && true == room.hasElapsedMs(3000) )
			{
#ifdef _DEBUG
				std::cout << "Room " << it->first << " gets started now.\n";
#endif
				for ( const auto idx : room.mParticipantS )
				{
					Client& participant = clientS[ idx ];
					Socket& participantSocket = participant.socket( );
					std::string data( participant.currentTetriminoInfo() );
					std::string curTet( TAG_CURRENT_TETRIMINO + std::to_string(data.size())
									   + TOKEN_SEPARATOR_2
										+ data );
					if ( -1 ==	participantSocket.sendOverlapped(curTet.data(), curTet.size()) )
					{
						// Exception
						if ( -1 == forceDisconnection( idx, "Failed to pend sending request to play to" ) )
						{
							// Break twice
							goto cleanUp;
						}
						break;
					}
					participantSocket.pend( );
					participant.setState( Client::State::PLAYING );
				}
				room.restartTimer( );
				room.setState( Room::State::PLAYING );
			}
		}

		if ( true == wasBoatful && 0 < candidateS.size() )
		{
			wasBoatful = false;
			const Index nextCandidateClientIdx = candidateS.front( );
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
			const Index evIdx = (Index)event.events[ i ].lpCompletionKey;
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