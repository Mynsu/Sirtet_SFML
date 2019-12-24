#include "pch.h"
#include "Client.h"
#include "Room.h"
#include <sstream>

Client::Client( const Socket::Type type, const ClientIndex index )
	: mIndex( index ), mState( State::UNVERIFIED ),
	mTicket( 0u ), mRoomID( -1 ),
	mRecentRequest( (Request)-1 ),
	mSocket( type )
{
}

bool Client::complete( std::vector<Client>& clientS,
					  std::vector<ClientIndex>& lobby,
					  std::unordered_map<HashedKey, Room>& roomS )
{
	const Socket::CompletedWork cmpl = mSocket.completedWork( );
	if ( Socket::CompletedWork::RECEIVE != cmpl &&
		Socket::CompletedWork::SEND != cmpl	)
	{
		// Exception
#ifdef _DEBUG
		__debugbreak( );
#else
		__assume( 0 );
#endif
	}
	const char* const rcvBuf = mSocket.receivingBuffer( );
	switch( mState )
	{
		case Client::State::IN_LOBBY:
			if ( Socket::CompletedWork::RECEIVE == cmpl )
			{
				const _Tag tag = (_Tag)*rcvBuf;
				if ( _Tag::REQUEST == tag )
				{
					const Request req = (Request)rcvBuf[TAG_REQUEST_LEN];
					switch ( req )
					{
						case Request::UPDATE_USER_LIST:
						{
							std::string userList;
							userList.reserve( lobby.size()*10 );
							for ( const ClientIndex idx : lobby )
							{
								const std::string& nickname = clientS[idx].nickname();
								const uint32_t nicknameLen = ::htonl((u_long)nickname.size());
								userList.append( (char*)&nicknameLen, sizeof(uint32_t) );
								userList += nickname;
							}
							Packet packet;
							packet.pack( TAGGED_REQ_UPDATE_USER_LIST, userList );
							if ( -1 == mSocket.sendOverlapped(packet) )
							{
								// Exception
								std::cerr << "Failed to notify Client "
									<< mIndex << " the list of users in the lobby.\n";
								return false;
							}
							mSocket.pend( );
							break;
						}
						case Request::CREATE_ROOM:
						{
							////
							// Generating random room ID
							////
							std::random_device rd;
							std::minstd_rand re( rd() );
							const RoomID roomID = (RoomID)re( );
							mRoomID = roomID;
							////
							roomS.emplace( roomID, mIndex );
							std::string response( TAGGED_REQ_CREATE_ROOM );
							if ( -1 == mSocket.sendOverlapped(response.data(), response.size()) )
							{
								// Exception
								std::cerr << "Failed to affirm Client "
									<< mIndex << "'s room creation.\n";
								return false;
							}
							mSocket.pend( );
							mRecentRequest = req;
							break;
						}
						case Request::JOIN_ROOM:
						{
							const uint32_t size	= ::ntohl(*(uint32_t*)&rcvBuf[TAGGED_REQ_JOIN_ROOM_LEN]);
							const std::string otherNickname( &rcvBuf[TAGGED_REQ_JOIN_ROOM_LEN+sizeof(uint32_t)], size );
							ResultJoiningRoom res = ResultJoiningRoom::NONE;
							if ( otherNickname == mNickname )
							{
								res = ResultJoiningRoom::FAILED_DUE_TO_SELF_TARGET;
							}
							else
							{
								auto cit = clientS.cbegin();
								while ( clientS.cend() != cit )
								{
									if ( otherNickname == cit->nickname() )
									{
										const RoomID roomID = cit->roomID();
										if ( auto rit = roomS.find(roomID);
											rit != roomS.end() )
										{
											mRoomID = roomID;
											res = (ResultJoiningRoom)rit->second.tryAccept( mIndex );
											if ( ResultJoiningRoom::SUCCCEDED == res )
											{
												mRecentRequest = req;
											}
										}
#ifdef _DEBUG
										// Failure
										else
										{
											res = ResultJoiningRoom::FAILED_BY_SERVER_ERROR;
											std::cerr << "Failed to help Client " << mIndex
												<< " join Room " << roomID << ", which doesn't exist at all.\n";
										}
#endif
										break;
									}
									++cit;
								}
								if ( clientS.cend() == cit )
								{
									res = ResultJoiningRoom::FAILED_DUE_TO_TARGET_NOT_CONNECTING;
								}
#ifdef _DEBUG
								// Failure
								if ( ResultJoiningRoom::NONE == res )
								{
									__debugbreak( );
								}
#endif
							}
							Packet packet;
							packet.pack( TAGGED_REQ_JOIN_ROOM, (uint32_t)res );
							if ( -1 == mSocket.sendOverlapped(packet) )
							{
								// Exception
								std::cerr << "Failed to tell if Client "
									<< mIndex << " can join the room or not.\n";
								return false;
							}
							mSocket.pend( );
							break;
						}
						// Exception
						default:
							std::cerr << "WARNING: Client "
								<< mIndex << " sent an undefined request.\n";
							if ( -1 == mSocket.receiveOverlapped() )
							{
								// Exception
								std::cerr << "Failed to pend reception from Client "
									<< mIndex << std::endl;
								return false;
							}
							mSocket.pend( );
							break;
					}
				}
				// Exception
				else
				{
					std::cerr << "WARNING: Client "
						<< mIndex << " sent data without a valid tag.\n";
					if ( -1 == mSocket.receiveOverlapped() )
					{
						// Exception
						std::cerr << "Failed to pend reception from Client "
							<< mIndex << std::endl;
						return false;
					}
					mSocket.pend( );
				}
			}
			else if ( Socket::CompletedWork::SEND == cmpl )
			{
				if ( Request::CREATE_ROOM == mRecentRequest ||
					Request::JOIN_ROOM == mRecentRequest )
				{
					mRecentRequest = Request::NONE;
					for ( auto it = lobby.cbegin(); lobby.cend() != it; ++it )
					{
						if ( mIndex == *it )
						{
							lobby.erase( it );
							break;
						}
					}
					for ( const ClientIndex idx : lobby )
					{
						Packet packet;
						packet.pack( TAGGED_NOTI_SOMEONE_LEFT, mNickname );
						Socket& _socket = clientS[idx].socket();
						if ( -1 == _socket.sendOverlapped(packet) )
						{
							// Exception
							std::cerr << "Failed to notify to Client "
								<< idx << " that " << mIndex << " left the lobby.\n";
#ifdef _DEBUG
							__debugbreak( );
#endif
							continue;
						}
						_socket.pend( );
					}
					mState = Client::State::WAITING_IN_ROOM;
				}

				if ( -1 == mSocket.receiveOverlapped() )
				{
					// Exception
					std::cerr << "Failed to pend reception from Client "
						<< mIndex << std::endl;
					return false;
				}
				mSocket.pend( );
			}
			break;
		case ::Client::State::WAITING_IN_ROOM:
			if ( ::Socket::CompletedWork::RECEIVE == cmpl )
			{
				const _Tag tag = (_Tag)*rcvBuf;
				if ( _Tag::REQUEST == tag )
				{
					const Request req = (Request)rcvBuf[TAG_REQUEST_LEN];
					switch ( req )
					{
						case Request::START_GAME:
							if ( Room& room = roomS[mRoomID];
								room.hostIndex() == mIndex )
							{
#ifdef _DEBUG
								std::cout << "Client "
									<< mIndex << " requested to create a room.\n";
#endif
								room.start( );
								mState = Client::State::PLAYING_IN_ROOM;
							}
							// Exception
							else
							{
								const int res = mSocket.disconnectOverlapped( );
								if ( 0 == res && ERROR_IO_PENDING != WSAGetLastError() )
								{
									// Exception
									std::cerr << "Failed to disconnect Client "
										<< mIndex << ".\n";
									return false;
								}
								else if ( -1 == res )
								{
									std::cerr << "FATAL: Failed to get DisconnectEx(...) for Client "
										<< mIndex << ".\n";
									return false;
								}
								else
								{
									mSocket.pend( );
								}
							}
							break;
						case Request::LEAVE_ROOM:
							if ( auto it = roomS.find(mRoomID); it != roomS.end() )
							{
								mRoomID = -1;
								mState = State::IN_LOBBY;
#ifdef _DEBUG
								std::cout << "Client "
									<< mIndex << " leaved Room " << it->first << ".\n";
#endif
								if ( false == it->second.leave(mIndex) )
								{
#ifdef _DEBUG
									std::cout << "Room "
										<< it->first << " has been destructed.\n";
#endif
									it = roomS.erase( it );
								}
								for ( const ClientIndex idx : lobby	)
								{
									Packet packet;
									packet.pack( TAGGED_NOTI_SOMEONE_VISITED, mNickname );
									Socket& _socket = clientS[idx].socket();
									if ( -1 == _socket.sendOverlapped(packet) )
									{
										// Exception
										std::cerr << "Failed to notify to Client "
											<< idx << " that " << mIndex << " came into the lobby.\n";
#ifdef _DEBUG
										__debugbreak( );
#endif
										continue;
									}
									_socket.pend( );
								}
								lobby.emplace_back( mIndex );
							}
#ifdef _DEBUG
							else
							{
								std::cerr << "Client "
									<< mIndex << " tried to leave the non-existent Room "
									<< mRoomID << ".\n";
							}
#endif
							if ( -1 == mSocket.receiveOverlapped() )
							{
								// Exception
								std::cerr << "Failed to pend reception from Client "
									<< mIndex << std::endl;
								return false;
							}
							mSocket.pend( );
							break;
						// Exception
						default:
							std::cerr << "WARNING: Client "
								<< mIndex << " sent an undefined request.\n";
							if ( -1 == mSocket.receiveOverlapped() )
							{
								// Exception
								std::cerr << "Failed to pend reception from Client "
									<< mIndex << std::endl;
								return false;
							}
							mSocket.pend( );
							break;
					}
				}
				// Exception
				else
				{
					std::cerr << "WARNING: Client "
						<< mIndex << " sent data without a valid tag.\n";
					if ( -1 == mSocket.receiveOverlapped() )
					{
						// Exception
						std::cerr << "Failed to pend reception from Client "
							<< mIndex << std::endl;
						return false;
					}
					mSocket.pend( );
				}
			}
			else if ( ::Socket::CompletedWork::SEND == cmpl )
			{
				if ( -1 == mSocket.receiveOverlapped() )
				{
					// Exception
					std::cerr << "Failed to pend reception from Client "
						<< mIndex << std::endl;
					return false;
				}
				mSocket.pend( );
			}
			break;
		case ::Client::State::PLAYING_IN_ROOM:
			if ( Socket::CompletedWork::RECEIVE == cmpl )
			{
				std::string_view strView( rcvBuf );
				bool hasException = true;
				if ( const size_t pos = strView.find(TAG_REQUEST); strView.npos != pos )
				{
					hasException = false;
					const Request req = (Request)rcvBuf[pos+TAG_REQUEST_LEN];
					switch ( req )
					{
						case Request::LEAVE_ROOM:
							if ( auto it = roomS.find(mRoomID); it != roomS.end() )
							{
								mRoomID = -1;
								mState = State::IN_LOBBY;
#ifdef _DEBUG
								std::cout << "Client "
									<< mIndex << " leaved Room " << it->first << ".\n";
#endif
								if ( false == it->second.leave(mIndex) )
								{
#ifdef _DEBUG
									std::cout << "Room "
										<< it->first << " has been destructed.\n";
#endif
									it = roomS.erase( it );
								}
								for ( const ClientIndex idx : lobby	)
								{
									Packet packet;
									packet.pack( TAGGED_NOTI_SOMEONE_VISITED, mNickname );
									Socket& _socket = clientS[idx].socket();
									if ( -1 == _socket.sendOverlapped(packet) )
									{
										// Exception
										std::cerr << "Failed to notify to Client "
											<< idx << " that " << mIndex << " came into the lobby.\n";
#ifdef _DEBUG
										__debugbreak( );
#endif
										continue;
									}
									_socket.pend( );
								}
								lobby.emplace_back( mIndex );
							}
							// Exception
							else
							{
								std::cerr << "Client "
									<< mIndex << " tried to leave the non-existent Room "
									<< mRoomID << ".\n";
#ifdef _DEBUG
								__debugbreak( );
#endif
							}

							if ( -1 == mSocket.receiveOverlapped() )
							{
								// Exception
								std::cerr << "Failed to pend reception from Client "
									<< mIndex << std::endl;
								return false;
							}
							mSocket.pend( );
							break;
						// Exception
						default:
							std::cerr << "WARNING: Client "
								<< mIndex << " sent an undefined request.\n";
							if ( -1 == mSocket.receiveOverlapped() )
							{
								// Exception
								std::cerr << "Failed to pend reception from Client "
									<< mIndex << std::endl;
								return false;
							}
							mSocket.pend( );
							break;
					}
					break;
				}

				if ( const size_t pos = strView.find(TAG_MY_TETRIMINO_MOVE); strView.npos != pos )
				{
					hasException = false;
					if ( auto it = roomS.find(mRoomID); roomS.end() != it )
					{
						const ::model::tetrimino::Move move =
							(::model::tetrimino::Move)*(uint8_t*)&rcvBuf[pos+TAG_MY_TETRIMINO_MOVE_LEN];
						it->second.perceive( mIndex, move );
					}
					// Exception
					else
					{
						std::cerr << "Client "
							<< mIndex << " tried to leave the non-existent Room "
							<< mRoomID << ".\n";
#ifdef _DEBUG
						__debugbreak( );
#endif
					}
				}

				if ( const size_t pos = strView.find(TAG_MY_TETRIMINO_COLLIDED_IN_CLIENT); strView.npos != pos )
				{
					hasException = false;
					if ( auto it = roomS.find(mRoomID); roomS.end() != it )
					{
						it->second.perceive( mIndex );
					}
					// Exception
					else
					{
						std::cerr << "Client "
							<< mIndex << " tried to leave the non-existent Room "
							<< mRoomID << ".\n";
#ifdef _DEBUG
						__debugbreak( );
#endif
					}
				}
				
				// Exception
				if ( true == hasException )
				{
					std::cerr << "WARNING: Client "
						<< mIndex << " sent data without a valid tag.\n";
				}
			}
			else if ( Socket::CompletedWork::SEND == cmpl )
			{
				if ( -1 == mSocket.receiveOverlapped() )
				{
					// Exception
					std::cerr << "Failed to pend reception from Client "
						<< mIndex << std::endl;
					return false;
				}
				mSocket.pend( );
			}
			break;
		default:
#ifdef _DEBUG
				__debugbreak( );
#else
				__assume( 0 );
#endif
				break;
	}

	return true;
}

Client::State Client::state() const
{
	return mState;
}

void Client::setState(const Client::State state)
{
	mState = state;
}

void Client::holdTicket(const Ticket ticket)
{
	mTicket = ticket;
}

RoomID Client::roomID( ) const
{
	return mRoomID;
}

void Client::setRoomID( const RoomID roomID )
{
	mRoomID = roomID;
}

const std::string& Client::nickname() const
{
	return mNickname;
}

void Client::setNickname( std::string& nickname )
{
	mNickname.clear( );
	mNickname = nickname;
}

Socket& Client::socket( )
{
	return mSocket;
}

void Client::setSocket( const Socket::Type type, const Socket::CompletedWork completedWork )
{
	mSocket.close( );
	mSocket = Socket( type, completedWork );
}

void Client::reset()
{
	mState = Client::State::UNVERIFIED;
	mTicket = 0u;
	mRoomID = -1;
	mRecentRequest = Request::NONE;
}