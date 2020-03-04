#include "pch.h"
#include "Client.h"
#include "Room.h"

const uint8_t NULL_HASHED_KEY = 0;
const uint8_t NULL_ROOM_ID = -1;
const Clock::duration MIN_REQ_GAP = std::chrono::milliseconds(100);
const Clock::duration MIN_MOVE_GAP = std::chrono::milliseconds(35);
const Clock::duration MIN_LAND_GAP = std::chrono::milliseconds(100);

Client::Client( const Socket::Type type, const ClientIndex index )
	: mIndex( index ), mState( State::UNVERIFIED ),
	mRoomID( NULL_ROOM_ID ), mNicknameHashed_( NULL_HASHED_KEY ),
	mSocket( type )
{
}

std::vector<ClientIndex> Client::work( const IOType completedIOType, 
									std::vector<Client>& clients,
									 std::vector<ClientIndex>& lobby,
									 std::unordered_map<HashedKey, Room>& rooms )
{
	ASSERT_TRUE ( IOType::RECEIVE == completedIOType || IOType::SEND == completedIOType	);
	std::vector<ClientIndex> failedIndices;
	const char* const rcvBuf = mSocket.receivingBuffer();
	const Clock::time_point now = Clock::now();
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
#ifdef _DEBUG
							std::cout << "Client " << mIndex <<
								" wants the list of users in the lobby.\n";
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
								packet.pack( TAGGED_REQ_USER_LIST, userList );
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
							if ( now-mTimeStamp[(int)TimeStampIndex::GENERAL] < MIN_REQ_GAP )
							{
								std::cerr << "Client " << mIndex <<
									" doesn't seem to be human's request.\n";
								failedIndices.emplace_back(mIndex);
								return failedIndices;
							}
							mTimeStamp[(int)TimeStampIndex::GENERAL] = now;
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
							std::random_device rd;
							std::minstd_rand re( rd() );
							RoomID roomID = NULL_ROOM_ID;
							while ( NULL_ROOM_ID == roomID ||
									rooms.end() != rooms.find(roomID) )
							{
								roomID = (RoomID)re();
							}
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
							if ( now-mTimeStamp[(int)TimeStampIndex::GENERAL] < MIN_REQ_GAP )
							{
								std::cerr << "Client " << mIndex <<
									" doesn't seem to be human's request.\n";
								failedIndices.emplace_back(mIndex);
								return failedIndices;
							}
							mTimeStamp[(int)TimeStampIndex::GENERAL] = now;
							const uint16_t size	= ::ntohs(
								*(uint16_t*)&rcvBuf[TAGGED_REQ_JOIN_ROOM_LEN]);
							const std::string targetNickname(
								&rcvBuf[TAGGED_REQ_JOIN_ROOM_LEN+sizeof(uint16_t)], size );
							ResultJoiningRoom res = ResultJoiningRoom::NONE;
							if ( targetNickname == mNickname )
							{
								res = ResultJoiningRoom::FAILED_DUE_TO_SELF_TARGET;
							}
							else
							{
								auto clientIt = clients.cbegin();
								while ( clients.cend() != clientIt )
								{
									if ( targetNickname == clientIt->nickname() )
									{
										const RoomID roomID = clientIt->roomID();
										auto roomIt = rooms.find(roomID);
										ASSERT_TRUE( roomIt != rooms.end() );
										res = (ResultJoiningRoom)roomIt->
											second.tryEmplace(mIndex);
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
										break;
									}
									++clientIt;
								}
								if ( clients.cend() == clientIt )
								{
									res = ResultJoiningRoom::FAILED_DUE_TO_TARGET_NOT_CONNECTING;
								}
								ASSERT_TRUE( ResultJoiningRoom::NONE != res );
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
							break;
					}
				}
				// Exception
				else
				{
					std::cerr << "WARNING: Client "
						<< mIndex << " sent data without a valid tag.\n";
				}

				if ( -1 == mSocket.receiveOverlapped() )
				{
					std::cerr << "Failed to receive from Client " <<
						mIndex << ".\n";
					failedIndices.emplace_back( mIndex );
					return failedIndices;
				}
			}
			break;
		case ::Client::State::WAITING_IN_ROOM:
			if ( IOType::RECEIVE == completedIOType )
			{
				const _Tag tag = (_Tag)*rcvBuf;
				if ( _Tag::REQUEST == tag )
				{
					mTimeStamp[(int)TimeStampIndex::GENERAL] = now;
					const Request req = (Request)rcvBuf[TAG_REQUEST_LEN];
					switch ( req )
					{
						case Request::UPDATE_USER_LIST:
						{
							auto it = rooms.find(mRoomID);
							ASSERT_TRUE( rooms.end() != it );
							it->second.perceive( mIndex, Room::Perception::INSTANTIATION_DONE );
							break;
						}
						case Request::START_GAME:
						{
							auto it = rooms.find(mRoomID);
							ASSERT_TRUE( rooms.end() != it && it->second.hostIndex() == mIndex );
#ifdef _DEBUG
							std::cout << "Client "
									<< mIndex << " starts the game.\n";
#endif
							it->second.start( );
							break;
						}
						case Request::LEAVE_ROOM:
						{
							auto it = rooms.find(mRoomID);
							ASSERT_TRUE( rooms.end() != it );
							mRoomID = NULL_ROOM_ID;
							mState = State::IN_LOBBY;
#ifdef _DEBUG
							std::cout << "Client "
								<< mIndex << " leaved Room " << it->first << ".\n";
#endif
							if ( 0 == it->second.pop(mIndex) )
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
							break;
						}
						// Exception
						default:
							std::cerr << "WARNING: Client "
								<< mIndex << " sent an undefined request.\n";
							break;
					}
				}
				else if ( _Tag::CURRENT_TETRIMINOS == tag )
				{
					auto it = rooms.find(mRoomID);
					ASSERT_TRUE( rooms.end() != it );
					it->second.perceive( mIndex, Room::Perception::INITIATION_DONE );
				}
				// Exception
				else
				{
					std::cerr << "WARNING: Client "
						<< mIndex << " sent data without a valid tag.\n";
				}

				if ( -1 == mSocket.receiveOverlapped() )
				{
					std::cerr << "Failed to receive from Client " <<
						mIndex << ".\n";
					failedIndices.emplace_back( mIndex );
					return failedIndices;
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
					if ( now-mTimeStamp[(int)TimeStampIndex::GENERAL] < MIN_REQ_GAP )
					{
						std::cerr << "Client " << mIndex <<
							" doesn't seem to be human's request.\n";
						failedIndices.emplace_back(mIndex);
						return failedIndices;
					}
					mTimeStamp[(int)TimeStampIndex::GENERAL] = now;
					hasException = false;
					const Request req = (Request)rcvBuf[pos+TAG_REQUEST_LEN];
					switch ( req )
					{
						case Request::LEAVE_ROOM:
						{
							auto it = rooms.find(mRoomID);
							ASSERT_TRUE( rooms.end() != it );
							mRoomID = NULL_ROOM_ID;
							mState = State::IN_LOBBY;
#ifdef _DEBUG
							std::cout << "Client "
								<< mIndex << " leaved Room " << it->first << ".\n";
#endif
							if ( 0 == it->second.pop(mIndex) )
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
							break;
						}
						// Exception
						default:
							std::cerr << "WARNING: Client "
								<< mIndex << " sent an undefined request.\n";
							break;
					}
				}

				bool hasCheckedTime = false;
				size_t pos = 0;
				while ( true )
				{
					pos = strView.find(TAG_MY_TETRIMINO_MOVE, pos);
					if ( strView.npos == pos )
					{
						break;
					}
					if ( false == hasCheckedTime )
					{
						if ( now-mTimeStamp[(int)TimeStampIndex::TETRIMINO_MOVED] < MIN_MOVE_GAP )
						{
							std::cerr << "Client " << mIndex <<
								" doesn't seem to be human's move.\n";
							failedIndices.emplace_back(mIndex);
							return failedIndices;
						}
						mTimeStamp[(int)TimeStampIndex::TETRIMINO_MOVED] = now;
						hasCheckedTime = true;
					}
					hasException = false;
					auto it = rooms.find(mRoomID);
					ASSERT_TRUE( rooms.end() != it );
					const ::model::tetrimino::Move move =
						(::model::tetrimino::Move)rcvBuf[pos+TAG_MY_TETRIMINO_MOVE_LEN];
					it->second.perceive( mIndex, move );
					++pos;
				}
				
				if ( pos = strView.find(TAG_MY_TETRIMINO_LANDED_ON_CLIENT);
					strView.npos != pos )
				{
					if ( now-mTimeStamp[(int)TimeStampIndex::TETRIMINO_LANDED] < MIN_LAND_GAP )
					{
						std::cerr << "Client " << mIndex <<
							" doesn't seem to be human's landing.\n";
						failedIndices.emplace_back(mIndex);
						return failedIndices;
					}
					mTimeStamp[(int)TimeStampIndex::TETRIMINO_LANDED] = now;
					hasException = false;
					auto it = rooms.find(mRoomID);
					ASSERT_TRUE( rooms.end() != it );
					it->second.perceive( mIndex, Room::Perception::TETRIMINO_LAND_ON_CLIENT );
				}

				if ( pos = strView.find(TAGGED_REQ_USER_LIST);
					strView.npos != pos	)
				{
					hasException = false;
					// Doing nothing.
				}

				if ( pos = strView.find(TAG_CURRENT_TETRIMINOS);
					strView.npos != pos	)
				{
					hasException = false;
					// Doing nothing.
				}

				// Exception
				if ( true == hasException )
				{
					std::cerr << "WARNING: Client "
					<< mIndex << " sent data without a valid tag.\n";
				}

				if ( -1 == mSocket.receiveOverlapped() )
				{
					std::cerr << "Failed to receive from Client " <<
						mIndex << ".\n";
					failedIndices.emplace_back( mIndex );
					return failedIndices;
				}
			}
			break;
		default:
#ifdef _DEBUG
				__debugbreak( );
#else
				__assume( 0 );
#endif
	}

	return failedIndices;
}

Client::State Client::state( ) const
{
	return mState;
}

void Client::setState(const Client::State state)
{
	mState = state;
}

RoomID Client::roomID( ) const
{
	return mRoomID;
}

void Client::setRoomID( const RoomID roomID )
{
	mRoomID = roomID;
}

const std::string& Client::nickname( ) const
{
	return mNickname;
}

HashedKey Client::nicknameHashed( ) const
{
	return mNicknameHashed_;
}

void Client::setNickname( std::string& nickname )
{
	mNickname.clear( );
	mNickname = nickname;
	mNicknameHashed_ = ::util::hash::Digest2(nickname);
}

Socket& Client::socket( )
{
	return mSocket;
}

void Client::reset( const bool isSocketReusable )
{
	mState = Client::State::UNVERIFIED;
	mRoomID = NULL_ROOM_ID;
	mNicknameHashed_ = NULL_HASHED_KEY;
	mNickname.clear( );
	if ( false == isSocketReusable )
	{
		mSocket.close( );
		mSocket.reset( false );
	}
	else
	{
		resetTimeStamp();
		mSocket.reset();
	}
}