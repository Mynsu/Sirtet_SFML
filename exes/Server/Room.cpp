#include "pch.h"
#include "Room.h"
#include "Client.h"

Room::Room( )
	: mHostIndex( -1 ), mState( State::WAITING )
{
	mParticipantS.reserve( PARTICIPANT_CAPACITY );
}

Room::Room( const ClientIndex hostIndex )
	: mHostIndex( hostIndex ), mState( State::WAITING )
{
	mParticipantS.emplace( hostIndex, Playing() );
}

void Room::start( )
{
	mState = State::STARTED;
}

bool Room::leave( const ClientIndex index )
{
	bool retVal = true;
	mParticipantS.erase( index );
	if ( true == mParticipantS.empty() )
	{
		retVal = false;
	}
	else if ( index == mHostIndex )
	{
		mHostIndex = mParticipantS.cbegin( )->first;
	}
	return retVal;
}

void Room::perceive( const ClientIndex index, const ::model::tetrimino::Move move )
{
	if ( auto it = mParticipantS.find(index); mParticipantS.end() != it )
	{
		it->second.perceive( move );
	}
	// Exception
	else
	{
		std::cerr << "Client " << index << " knows the incorrect room ID.\n";
	}
}

void Room::perceive( const ClientIndex index, const bool async )
{
	if ( auto it = mParticipantS.find(index); mParticipantS.end() != it )
	{
		it->second.synchronize( async );
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
		for ( auto& it : mParticipantS )
		{
			// Exception
			if ( false == it.second.update() )
			{
				retVal.emplace_front( it.first );
				std::cerr << "FAIL: Client " << it.first << "'s timed out.\n";
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
			for ( auto& it : mParticipantS )
			{
				const ClientIndex participantIdx = it.first;
				Socket& participantSocket = clientS[ participantIdx ].socket( );
				std::string response( TAGGED_REQ_GET_READY );
				if ( -1 == participantSocket.sendOverlapped(response.data(), response.size()) )
				{
					// Exception
					std::cerr << "Failed to notify Client " << participantIdx
						<< " that the game get started.\n";
					retVal.emplace_front( participantIdx );
				}
				participantSocket.pend( );
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
				for ( auto& it : mParticipantS )
				{
					const ClientIndex participantIdx = it.first;
					Socket& participantSocket = clientS[ participantIdx ].socket( );
					Playing& participantPlay = it.second;
					std::string tetOnNet( participantPlay.tetriminoOnNet() );
					std::string tempoOnNet( participantPlay.tempoMsOnNet() );
					Packet packet;
					packet.pack( TAG_MY_CURRENT_TETRIMINO, tetOnNet );
					packet.pack( TAG_MY_TEMPO_MS, tempoOnNet );
					if ( -1 == participantSocket.sendOverlapped(packet) )
					{
						// Exception
						std::cerr << "Failed to send the current tetrimino to Client "
							<< participantIdx << ".\n";
						retVal.emplace_front( participantIdx );
						continue;
					}
					participantSocket.pend( );
				}
				mState = Room::State::PLAYING;
			}
			break;
		case Room::State::PLAYING:
			for ( auto& it : mParticipantS )
			{
				const ClientIndex participantIdx = it.first;
				Socket& participantSocket = clientS[ participantIdx ].socket( );
				if ( const Playing::UpdateResult res = it.second.updateResult();
					Playing::UpdateResult::TETRIMINO_LANDED == res )
				{
					Packet packet;
					Playing& participantPlay = it.second;
					std::string tetriminoOnNet( participantPlay.tetriminoOnNet() );
					packet.pack( TAG_MY_CURRENT_TETRIMINO, tetriminoOnNet );
					std::string tempoOnNet( participantPlay.tempoMsOnNet() );
					packet.pack( TAG_MY_TEMPO_MS, tempoOnNet );
					std::string stageOnNet( participantPlay.stageOnNet() );
					packet.pack( TAG_MY_STAGE, stageOnNet );

					if ( -1 == participantSocket.sendOverlapped(packet) )
					{
						// Exception
						std::cerr << "Failed to send the current info to Client "
							<< participantIdx << ".\n";
						retVal.emplace_front( participantIdx );
						continue;
					}
					participantSocket.pend( );
				}
				else
				{
					if ( false == participantSocket.isPending() )
					{
						if ( -1 == participantSocket.receiveOverlapped() )
						{
							// Exception
							std::cerr << "Failed to pend reception from Client "
								<< participantIdx << std::endl;
							retVal.emplace_front( participantIdx );
						}
						participantSocket.pend( );
					}
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