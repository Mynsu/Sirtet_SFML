#include "pch.h"
#include "Client.h"
#include "Room.h"
#include <sstream>

Client::Client( const Socket::Type type, const ClientIndex index )
	: mIndex( index ), mState( State::UNVERIFIED ),
	mTicket( 0u ), mRoomID( -1 ),
	mNicknameHashed_( 0 ),
	mSocket( type )
{
}

std::vector<ClientIndex> Client::work( const IOType completedIOType, 
									std::vector<Client>& clients,
									 std::vector<ClientIndex>& lobby,
									 std::unordered_map<HashedKey, Room>& rooms )
{
#ifdef _DEBUG
	if ( IOType::RECEIVE != completedIOType && 
		IOType::SEND != completedIOType	)
	{
		__debugbreak( );
	}
#endif
	std::vector<ClientIndex> failedIndices;
	const char* const rcvBuf = mSocket.receivingBuffer( );
	switch( mState )
	{
		case Client::State::IN_LOBBY:
			if ( IOType::RECEIVE == completedIOType )
			{
				const _Tag tag = (_Tag)*rcvBuf;
				if ( _Tag::REQUEST == tag )
				{
					const Request req = (Request)rcvBuf[TAG_REQUEST_LEN];
					switch ( req )
					{
						case Request::UPDATE_USER_LIST:
						{
							if ( -1 == mSocket.receiveOverlapped() )
							{
								std::cerr << "Failed to receive from Client " <<
									mIndex << ".\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
#ifdef _DEBUG
							std::cout << "Client " << mIndex <<
								"wants the list of users in the lobby.\n";
#endif
							std::string userList;
							userList.reserve( lobby.size()*10 );
							for ( const ClientIndex idx : lobby )
							{
								const std::string& nickname = clients[idx].nickname();
								const uint8_t nicknameLen = (uint8_t)nickname.size();
								userList += (char)nicknameLen;
								userList += nickname;
							}
							if ( false == userList.empty() )
							{
								Packet packet;
								packet.pack( TAGGED_REQ_USER_LIST_IN_LOBBY, userList );
								if ( -1 == mSocket.sendOverlapped(packet) )
								{
									// Exception
									std::cerr << "Failed to notify Client "
										<< mIndex << " the list of users in the lobby.\n";
									failedIndices.emplace_back( mIndex );
									return failedIndices;
								}
							}
							break;
						}
						case Request::CREATE_ROOM:
						{
							if ( -1 == mSocket.receiveOverlapped() )
							{
								std::cerr << "Failed to receive from Client " <<
									mIndex << ".\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
							// NOTE: Not checking out ...
							// ... if there's memory space enough to create one more room.
#ifdef _DEBUG
							std::cout << "Client " << mIndex << " created a room.\n";
#endif
							for ( auto it = lobby.cbegin(); lobby.cend() != it; ++it )
							{
								if ( mIndex == *it )
								{
									lobby.erase( it );
									break;
								}
							}
							mState = Client::State::WAITING_IN_ROOM;
							////
							// Generating random room ID
							////
							std::random_device rd;
							std::minstd_rand re( rd() );
							const RoomID roomID = (RoomID)re();
							////
							mRoomID = roomID;
							rooms.emplace( roomID, mIndex );
							std::string response( TAGGED_REQ_CREATE_ROOM );
							if ( -1 == mSocket.sendOverlapped(response.data(), response.size()) )
							{
								// Exception
								std::cerr << "Failed to affirm Client "
									<< mIndex << "'s room creation.\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
							break;
						}
						case Request::JOIN_ROOM:
						{
							const uint32_t size	= ::ntohl(
								*(uint32_t*)&rcvBuf[TAGGED_REQ_JOIN_ROOM_LEN]);
							const std::string targetNickname(
								&rcvBuf[TAGGED_REQ_JOIN_ROOM_LEN+sizeof(uint32_t)], size );
							if ( -1 == mSocket.receiveOverlapped() )
							{
								std::cerr << "Failed to receive from Client " <<
									mIndex << ".\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
							ResultJoiningRoom res = ResultJoiningRoom::NONE;
							if ( targetNickname == mNickname )
							{
								res = ResultJoiningRoom::FAILED_DUE_TO_SELF_TARGET;
							}
							else
							{
								auto cit = clients.cbegin();
								while ( clients.cend() != cit )
								{
									if ( targetNickname == cit->nickname() )
									{
										const RoomID roomID = cit->roomID();
										if ( auto rit = rooms.find(roomID);
											rit != rooms.end() )
										{
											res = (ResultJoiningRoom)rit->
												second.tryAccept(mIndex);
											if ( ResultJoiningRoom::SUCCCEDED == res )
											{
												for ( auto it = lobby.cbegin();
													 lobby.cend() != it; ++it )
												{
													if ( mIndex == *it )
													{
														lobby.erase( it );
														break;
													}
												}
												mState = Client::State::WAITING_IN_ROOM;
												mRoomID = roomID;
											}
										}
#ifdef _DEBUG
										// Failure
										else
										{
											__debugbreak( );
										}
#endif
										break;
									}
									++cit;
								}
								if ( clients.cend() == cit )
								{
									res = ResultJoiningRoom::FAILED_DUE_TO_TARGET_NOT_CONNECTING;
								}
#ifdef _DEBUG
								// Failure
								else if ( ResultJoiningRoom::NONE == res )
								{
									__debugbreak( );
								}
#endif
							}
							Packet packet;
							packet.pack( TAGGED_REQ_JOIN_ROOM, (uint8_t)res );
							if ( -1 == mSocket.sendOverlapped(packet) )
							{
								// Exception
								std::cerr << "Failed to tell if Client "
									<< mIndex << " can join the room or not.\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
#ifdef _DEBUG
							else
							{
								std::cout << "Client " << mIndex <<
									" tried to join the room where " << targetNickname <<
									" is and got the result(" << (int)res << ").\n";
							}
#endif
							break;
						}
						// Exception
						default:
							std::cerr << "WARNING: Client "
								<< mIndex << " sent an undefined request.\n";
							if ( -1 == mSocket.receiveOverlapped() )
							{
								std::cerr << "Failed to receive from Client " <<
									mIndex << ".\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
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
						std::cerr << "Failed to receive from Client " <<
							mIndex << ".\n";
						failedIndices.emplace_back( mIndex );
						return failedIndices;
					}
				}
			}
			break;
		case ::Client::State::WAITING_IN_ROOM:
			if ( IOType::RECEIVE == completedIOType )
			{
				const _Tag tag = (_Tag)*rcvBuf;
				if ( _Tag::REQUEST == tag )
				{
					const Request req = (Request)rcvBuf[TAG_REQUEST_LEN];
					switch ( req )
					{
						case Request::START_GAME:
							if ( -1 == mSocket.receiveOverlapped() )
							{
								std::cerr << "Failed to receive from Client " <<
									mIndex << ".\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
							if ( auto it = rooms.find(mRoomID);
								rooms.end() != it &&
								it->second.hostIndex() == mIndex )
							{
#ifdef _DEBUG
								std::cout << "Client "
									<< mIndex << " starts the game.\n";
#endif
								it->second.start( );
							}
#ifdef _DEBUG
							else
							{
								__debugbreak( );
							}
#endif
							break;
						case Request::LEAVE_ROOM:
							if ( -1 == mSocket.receiveOverlapped() )
							{
								std::cerr << "Failed to receive from Client " <<
									mIndex << ".\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
							if ( auto it = rooms.find(mRoomID);
								it != rooms.end() )
							{
								mRoomID = -1;
								mState = State::IN_LOBBY;
#ifdef _DEBUG
								std::cout << "Client "
									<< mIndex << " leaved Room " << it->first << ".\n";
#endif
								if ( 0 == it->second.leave(mIndex) )
								{
#ifdef _DEBUG
									std::cout << "Room "
										<< it->first << " has been destructed.\n";
#endif
									rooms.erase( it );
								}
								lobby.emplace_back( mIndex );
								Packet packet;
								const uint8_t ignored = 1;
								packet.pack(TAGGED_REQ_LEAVE_ROOM, ignored );
								if ( -1 == mSocket.sendOverlapped(packet) )
								{
									std::cerr << "Failed to tell Client " <<
										mIndex << " leaved the room.\n";
									failedIndices.emplace_back( mIndex );
									return failedIndices;
								}
							}
#ifdef _DEBUG
							else
							{
								__debugbreak( );
							}
#endif
							break;
						// Exception
						default:
							std::cerr << "WARNING: Client "
								<< mIndex << " sent an undefined request.\n";
							if ( -1 == mSocket.receiveOverlapped() )
							{
								std::cerr << "Failed to receive from Client " <<
									mIndex << ".\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
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
						std::cerr << "Failed to receive from Client " <<
							mIndex << ".\n";
						failedIndices.emplace_back( mIndex );
						return failedIndices;
					}
				}
			}
			break;
		case ::Client::State::PLAYING_IN_ROOM:
			if ( IOType::RECEIVE == completedIOType )
			{
				std::string_view strView( rcvBuf );
				bool hasException = true;
				if ( const size_t pos = strView.find(TAG_REQUEST);
					strView.npos != pos )
				{
					hasException = false;
					const Request req = (Request)rcvBuf[pos+TAG_REQUEST_LEN];
					switch ( req )
					{
						case Request::LEAVE_ROOM:
							if ( -1 == mSocket.receiveOverlapped() )
							{
								std::cerr << "Failed to receive from Client " <<
									mIndex << ".\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
							if ( auto it = rooms.find(mRoomID);
								it != rooms.end() )
							{
								mRoomID = -1;
								mState = State::IN_LOBBY;
#ifdef _DEBUG
								std::cout << "Client "
									<< mIndex << " leaved Room " << it->first << ".\n";
#endif
								if ( 0 == it->second.leave(mIndex) )
								{
#ifdef _DEBUG
									std::cout << "Room "
										<< it->first << " has been destructed.\n";
#endif
									rooms.erase( it );
								}
								lobby.emplace_back( mIndex );
								Packet packet;
								const uint8_t ignored = 1;
								packet.pack(TAGGED_REQ_LEAVE_ROOM, ignored );
								if ( -1 == mSocket.sendOverlapped(packet) )
								{
									std::cerr << "Failed to tell Client " <<
										mIndex << " leaved the room.\n";
									failedIndices.emplace_back( mIndex );
									return failedIndices;
								}
							}
#ifdef _DEBUG
							else
							{
								__debugbreak( );
							}
#endif
							break;
						// Exception
						default:
							std::cerr << "WARNING: Client "
								<< mIndex << " sent an undefined request.\n";
							if ( -1 == mSocket.receiveOverlapped() )
							{
								std::cerr << "Failed to receive from Client " <<
									mIndex << ".\n";
								failedIndices.emplace_back( mIndex );
								return failedIndices;
							}
							break;
					}
					break;
				}

				if ( const size_t pos = strView.find(TAG_MY_TETRIMINO_MOVE);
					strView.npos != pos )
				{
					hasException = false;
					if ( auto it = rooms.find(mRoomID);
						rooms.end() != it )
					{
						const ::model::tetrimino::Move move =
							(::model::tetrimino::Move)rcvBuf[pos+TAG_MY_TETRIMINO_MOVE_LEN];
						it->second.perceive( mIndex, move );
					}
#ifdef _DEBUG
					else
					{
						__debugbreak( );
					}
#endif
				}

				if ( const size_t pos = strView.find(TAG_MY_TETRIMINO_COLLIDED_ON_CLIENT);
					strView.npos != pos )
				{
					hasException = false;
					if ( auto it = rooms.find(mRoomID);
						rooms.end() != it )
					{
						it->second.perceive( mIndex );
					}
#ifdef _DEBUG
					else
					{
						__debugbreak( );
					}
#endif
				}

				if ( -1 == mSocket.receiveOverlapped() )
				{
					std::cerr << "Failed to receive from Client " <<
						mIndex << ".\n";
					failedIndices.emplace_back( mIndex );
					return failedIndices;
				}
				
				// Exception
				if ( true == hasException )
				{
					std::cerr << "WARNING: Client "
					<< mIndex << " sent data without a valid tag.\n";
				}
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

	return failedIndices;
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

HashedKey Client::nicknameHashed() const
{
	return mNicknameHashed_;
}

void Client::setNickname( std::string& nickname )
{
	mNickname.clear( );
	mNickname = nickname;
	mNicknameHashed_ = ::util::hash::Digest( nickname.data(), (uint8_t)nickname.size() );
}

Socket& Client::socket( )
{
	return mSocket;
}

void Client::reset( const bool isSocketReusable )
{
	mState = Client::State::UNVERIFIED;
	mTicket = 0u;
	mRoomID = -1;
	mNicknameHashed_ = 0;
	mNickname.clear( );
	if ( false == isSocketReusable )
	{
		mSocket.close( );
		mSocket = Socket(Socket::Type::TCP);
	}
}