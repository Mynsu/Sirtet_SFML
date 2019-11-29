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
	switch( mState )
	{
		case Client::State::IN_LOBBY:
			if ( Socket::CompletedWork::RECEIVE == cmpl )
			{
				const char* const rcvBuf = mSocket.receivingBuffer( );
				const Request req = (Request)std::atoi( rcvBuf );
				switch ( req )
				{
					case Request::REQ_CREATE_ROOM:
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
						std::string response( TAG_REQ_CREATE_ROOM );
						if ( -1 == mSocket.sendOverlapped(response.data(), response.size()) )
						{
							// Exception
							std::cerr << "Failed to affirm Client " << mIndex << "'s room creation";
							return false;
						}
						mSocket.pend( );
						mRecentRequest = req;
						break;
					}
					default:
						std::cerr << "WARNING: Client " << mIndex << " sent an undefined request.\n";
						if ( -1 == mSocket.receiveOverlapped() )
						{
							// Exception
							std::cerr << "Failed to pend reception from Client " << mIndex << std::endl;
							return false;
						}
						mSocket.pend( );
						break;
				}
			}
			else if ( Socket::CompletedWork::SEND == cmpl )
			{
				switch( mRecentRequest )
				{
					case Request::REQ_CREATE_ROOM:
						mState = Client::State::IN_ROOM;
						if ( -1 == mSocket.receiveOverlapped() )
						{
							// Exception
							std::cerr << "Failed to pend reception from Client " << mIndex << std::endl;
							return false;
						}
						mSocket.pend( );
						break;
					default:
						break;
				}
			}
			// Exception
			else
			{
#ifdef _DEBUG
					__debugbreak( );
#else
					__assume( 0 );
#endif
			}
			break;
		case ::Client::State::IN_ROOM:
			if ( ::Socket::CompletedWork::RECEIVE == cmpl )
			{
				const char* const rcvBuf = mSocket.receivingBuffer( );
				const Request req = (Request)std::atoi(rcvBuf);
				switch ( req )
				{
					case Request::REQ_START_GAME:
						if ( Room& room = roomS[mRoomID];
							room.hostIndex() == mIndex )
						{
#ifdef _DEBUG
							std::cout << "Client " << mIndex << " requested to create a room.\n";
#endif
							room.start( );
						}
						// Exception
						else
						{
							const int res = mSocket.disconnectOverlapped( );
							if ( 0 == res && ERROR_IO_PENDING != WSAGetLastError() )
							{
								// Exception
								std::cerr << "Failed to disconnect Client " << mIndex << ".\n";
								return false;
							}
							else if ( -1 == res )
							{
								std::cerr << "FATAL: Failed to get DisconnectEx(...) for Client " << mIndex << ".\n";
								return false;
							}
							else
							{
								mSocket.pend( );
							}
						}
						break;
//										case Request::INVITE:
//										{
//											const ClientIndex guestIdx = (ClientIndex)std::atoi(&rcvBuf[TAG_INVITE_LEN]);
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
						std::cerr << "WARNING: Tried to disconnect Client " << mIndex << " with the undefined behavior.\n";
						const int res = mSocket.disconnectOverlapped( );
						if ( 0 == res && ERROR_IO_PENDING != WSAGetLastError() )
						{
							// Exception
							std::cerr << "Failed to disconnect Client " << mIndex << ".\n";
							return false;
						}
						else if ( -1 == res )
						{
							std::cerr << "FATAL: Failed to get DisconnectEx(...) for Client " << mIndex << ".\n";
							return false;
						}
						else
						{
							mSocket.pend( );
						}
						break;
					}
				}
			}
			else if ( ::Socket::CompletedWork::SEND == cmpl )
			{

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
////�ñ�: full-duplex �ɱ�?  �� �ƾ��µ�.
//											if ( -1 == participantSocket.sendOverlapped(noti.data(), noti.size()) )
//											{
//												// Exception
//												if ( -1 == forceDisconnection( clientIdx, "Failed to notify joining to" ) )
//												{
//													// Break twice
//													goto cleanUp;
//												}
////TODO: ���� �˸���												
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
////TODO: ���� �˸���											
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

	return true;
}

void Client::setState(const Client::State state)
{
	mState = state;
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
	mRecentRequest = Request::REQ_NULL;
}

Client::State Client::state() const
{
	return mState;
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