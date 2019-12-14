#include "pch.h"
#include "Room.h"
#include "Client.h"

Room::Room( )
	: mHostIndex( -1 ), mState( State::WAITING )
{
	mGuestS.reserve( PARTICIPANT_CAPACITY );
}

Room::Room( const ClientIndex hostIndex )
	: mHostIndex( hostIndex ), mState( State::WAITING )
{
	mGuestS.emplace( hostIndex, Playing() );
}

void Room::start( )
{
	mState = State::STARTED;
}

bool Room::leave( const ClientIndex index )
{
	bool retVal = true;
	mGuestS.erase( index );
	if ( true == mGuestS.empty() )
	{
		retVal = false;
	}
	else if ( index == mHostIndex )
	{
		mHostIndex = mGuestS.cbegin( )->first;
	}
	return retVal;
}

void Room::perceive( const ClientIndex index, const ::model::tetrimino::Move move )
{
	if ( auto it = mGuestS.find(index); mGuestS.end() != it )
	{
		it->second.perceive( move );
	}
	// Exception
	else
	{
		std::cerr << "Client " << index << " knows the incorrect room ID.\n";
	}
}

void Room::perceive( const ClientIndex index, const bool hasTetriminoCollidedInClient )
{
	if ( auto it = mGuestS.find(index); mGuestS.end() != it )
	{
		it->second.perceive( );
	}
	// Exception
	else
	{
		std::cerr << "Client " << index << " knows the incorrect room ID.\n";
	}
}

std::forward_list<ClientIndex> Room::update( )
{
	std::forward_list<ClientIndex> retVal;
	if ( Room::State::PLAYING == mState )
	{
		for ( auto& it : mGuestS )
		{
			// Exception
			if ( false == it.second.update() )
			{
				retVal.emplace_front( it.first );
				std::cerr << "FAIL: Client " << it.first << "'s time out.\n";
			}
		}
	}
	return retVal;
}

std::forward_list<ClientIndex> Room::notify( std::vector<Client>& clientS )
{
	std::forward_list<ClientIndex> retVal;
	switch ( mState )
	{
		case Room::State::WAITING:
			//TODO
			break;
		case Room::State::STARTED:
			for ( auto& it : mGuestS )
			{
				const ClientIndex guestIdx = it.first;
				Socket& guestSocket = clientS[ guestIdx ].socket( );
				std::string response( TAGGED_REQ_GET_READY );
				if ( -1 == guestSocket.sendOverlapped(response.data(), response.size()) )
				{
					// Exception
					std::cerr << "Failed to notify Client " << guestIdx
						<< " that the game get started.\n";
					retVal.emplace_front( guestIdx );
				}
				guestSocket.pend( );
				it.second.spawnTetrimino( );
			}
			mStartTime = Clock::now( );
			mState = Room::State::READY;
			break;
		case Room::State::READY:
			if ( true == alarmAfter(3000) )
			{
#ifdef _DEBUG
				std::cout << "Room hosted by " << mHostIndex << " gets started now.\n";
#endif
				for ( auto& it : mGuestS )
				{
					const ClientIndex guestIdx = it.first;
					Socket& guestSocket = clientS[ guestIdx ].socket( );
					Playing& guestPlay = it.second;
					Packet packet;
					const ::model::tetrimino::Type curTetType = guestPlay.currentTetriminoType();
					packet.pack( TAG_MY_CURRENT_TETRIMINO, (uint32_t)curTetType );
					const ::model::tetrimino::Type nextTetType = guestPlay.nextTetriminoType();
					packet.pack( TAG_MY_NEXT_TETRIMINO, (uint32_t)nextTetType );
					const uint32_t tempoMs = guestPlay.tempoMs();
					packet.pack( TAG_MY_TEMPO_MS, tempoMs );
					if ( -1 == guestSocket.sendOverlapped(packet) )
					{
						// Exception
						std::cerr << "Failed to send the current tetrimino to Client "
							<< guestIdx << ".\n";
						retVal.emplace_front( guestIdx );
						continue;
					}
					guestSocket.pend( );
				}
				mState = Room::State::PLAYING;
			}
			break;
		case Room::State::PLAYING:
			for ( auto& it : mGuestS )
			{
				const ClientIndex guestIdx = it.first;
				Socket& guestSocket = clientS[ guestIdx ].socket( );
				const Playing::UpdateResult res = it.second.updateResult( );
				switch ( res )
				{
					case Playing::UpdateResult::TETRIMINO_LANDED:
					{
						Packet packet;
						Playing& guestPlay = it.second;
						const ::model::tetrimino::Type nextTetType = guestPlay.nextTetriminoType();
						packet.pack( TAG_MY_NEXT_TETRIMINO, (uint32_t)nextTetType );
						std::string stageOnNet( guestPlay.stageOnNet() );
						packet.pack( TAG_MY_STAGE, stageOnNet );
						if ( -1 == guestSocket.sendOverlapped(packet) )
						{
							// Exception
							std::cerr << "Failed to send the current info to Client "
								<< guestIdx << ".\n";
							retVal.emplace_front( guestIdx );
							continue;
						}
						guestSocket.pend( );
						break;
					}
					case Playing::UpdateResult::LINE_CLEARED:
					{
						Packet packet;
						Playing& guestPlay = it.second;
						const ::model::tetrimino::Type nextTetType = guestPlay.nextTetriminoType();
						packet.pack( TAG_MY_NEXT_TETRIMINO, (uint32_t)nextTetType );
						std::string stageOnNet( guestPlay.stageOnNet() );
						packet.pack( TAG_MY_STAGE, stageOnNet );
						const uint32_t tempoMs = guestPlay.tempoMs();
						packet.pack( TAG_MY_TEMPO_MS, tempoMs );
						const uint8_t numOfLinesCleared = guestPlay.numOfLinesCleared();
						packet.pack( TAG_MY_LINES_CLEARED, numOfLinesCleared );
						if ( -1 == guestSocket.sendOverlapped(packet) )
						{
							// Exception
							std::cerr << "Failed to send the current info to Client "
								<< guestIdx << ".\n";
							retVal.emplace_front( guestIdx );
							continue;
						}
						guestSocket.pend( );
						break;
					}
					case Playing::UpdateResult::GAME_OVER:
					{
						Packet packet;
						Playing& guestPlay = it.second;
						std::string stageOnNet( guestPlay.stageOnNet() );
						packet.pack( TAG_MY_STAGE, stageOnNet );
						const uint8_t ignored = 1;
						packet.pack( TAG_MY_GAME_OVER, ignored );
						if ( -1 == guestSocket.sendOverlapped(packet) )
						{
							// Exception
							std::cerr << "Failed to send the current info to Client "
								<< guestIdx << ".\n";
							retVal.emplace_front( guestIdx );
							continue;
						}
						guestSocket.pend( );
						break;
					}
					default:
						if ( false == guestSocket.isPending() )
						{
							if ( -1 == guestSocket.receiveOverlapped() )
							{
								// Exception
								std::cerr << "Failed to pend reception from Client "
									<< guestIdx << std::endl;
								retVal.emplace_front( guestIdx );
							}
							guestSocket.pend( );
						}
						break;
				}
			}
			break;
		default:
			break;
	}
	return retVal;
}

ClientIndex Room::hostIndex( ) const
{
	return mHostIndex;
}