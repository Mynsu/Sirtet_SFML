#include "pch.h"
#include "Client.h"
#include "Room.h"

Client::Client( const Socket::Type type, const ClientIndex index )
	: mIndex( index ), mState( State::UNVERIFIED ),
	mTicket( 0u ), mRoomID( -1 ),
	mRecentRequest( (Request)-1 ),
	mSocket( type )
{
}

bool Client::complete( std::unordered_map<HashedKey, Room>& roomS )
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
				const _Tag tag = (_Tag)std::atoi( rcvBuf );
				if ( _Tag::REQUEST == tag )
				{
					const Request req = (Request)std::atoi( &rcvBuf[TAG_REQUEST_LEN] );
					switch ( req )
					{
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
									<< mIndex << "'s room creation";
								return false;
							}
							mSocket.pend( );
							mRecentRequest = req;
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
				switch( mRecentRequest )
				{
					case Request::CREATE_ROOM:
						mState = Client::State::WAITING_IN_ROOM;
						if ( -1 == mSocket.receiveOverlapped() )
						{
							// Exception
							std::cerr << "Failed to pend reception from Client "
								<< mIndex << std::endl;
							return false;
						}
						mSocket.pend( );
						break;
					default:
						break;
				}
			}
			break;
		case ::Client::State::WAITING_IN_ROOM:
			if ( ::Socket::CompletedWork::RECEIVE == cmpl )
			{
				const _Tag tag = (_Tag)std::atoi( rcvBuf );
				if ( _Tag::REQUEST == tag )
				{
					const Request req = (Request)std::atoi( &rcvBuf[TAG_REQUEST_LEN] );
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
				const _Tag tag = (_Tag)std::atoi( rcvBuf );
				switch ( tag )
				{
					case _Tag::REQUEST:
					{
						const Request req = (Request)std::atoi( &rcvBuf[TAG_REQUEST_LEN] );
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
						break;
					}
					case _Tag::TETRIMINO_MOVE:
					{
						const ::model::tetrimino::Move move =
							(::model::tetrimino::Move)std::atoi( &rcvBuf[TAG_TETRIMINO_MOVE_LEN] );
						if ( auto it = roomS.find(mRoomID); roomS.end() != it )
						{
							it->second.perceive( mIndex, move );
						}
						// Exception
						else
						{
							std::cerr << "The room where Client "
								<< mIndex << " thinks they are doesn't exist.\n";
#ifdef _DEBUG
							__debugbreak( );
#endif
						}
						break;
					}
					case _Tag::SYNC:
						if ( auto it = roomS.find(mRoomID); roomS.end() != it )
						{
							const bool async = (bool)std::atoi( &rcvBuf[TAG_SYNC_LEN] );
							it->second.perceive( mIndex, async );
						}
						break;
					// Exception
					default:
					{
						std::cerr << "WARNING: Client "
							<< mIndex << " sent data without a valid tag.\n";
					}
					break;
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