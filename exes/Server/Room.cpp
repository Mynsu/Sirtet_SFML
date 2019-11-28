#include "pch.h"
#include "Room.h"
#include "Client.h"

Room::Room( )
	: mHostIndex( -1 ), mState( State::WAITING )
{
}

Room::Room( const ClientIndex hostIndex )
	: mHostIndex( hostIndex ), mState( State::WAITING )
{
	mParticipantS.emplace( hostIndex, Playing() );
}

std::forward_list<ClientIndex> Room::update( std::vector<Client>& clientS )
{
	std::forward_list<ClientIndex> retVal;
	switch ( mState )
	{
		case Room::State::WAITING:
			break;
		case Room::State::STARTED:
			for ( auto& it : mParticipantS )
			{
				const ClientIndex participantIdx = it.first;
				Socket& participantSocket = clientS[ participantIdx ].socket( );
				std::string req( TAG_REQ_GET_READY );
				if ( -1 == participantSocket.sendOverlapped(req.data(), req.size()) )
				{
					// Exception
					std::cerr << "Failed to notify Client " << participantIdx
						<< " that the game get started.\n";
					retVal.emplace_front( participantIdx );
				}
				participantSocket.pend( );
				it.second.spawnTetrimino( );
			}
			restartTimer( );
			mState = Room::State::READY;
			break;
		case Room::State::READY:
			if ( true == hasElapsedMs(3000) )
			{
#ifdef _DEBUG
				std::cout << "Room hosted by " << mHostIndex << " gets started now.\n";
#endif
				for ( auto& it : mParticipantS )
				{
					const ClientIndex participantIdx = it.first;
					Socket& participantSocket = clientS[ participantIdx ].socket( );
					Playing& participantPlay = it.second;
					std::string data( participantPlay.currentTetriminoInfo() );
					std::string curTet( TAG_MY_CURRENT_TETRIMINO + std::to_string(data.size())
									   + TOKEN_SEPARATOR_2
									   + data );
					if ( -1 == participantSocket.sendOverlapped(curTet.data(), curTet.size()) )
					{
						// Exception
						std::cerr << "Failed to send the current tetrimino to Client "
							<< participantIdx << ".\n";
						retVal.emplace_front( participantIdx );
						continue;
					}
					participantSocket.pend( );
				}
				restartTimer( );
				mState = Room::State::PLAYING;
			}
			break;
		case Room::State::PLAYING:
			for ( auto& it : mParticipantS )
			{
				if ( const Playing::Change res = it.second.update(); true == res )
				{
					Packet packet;
					Playing& participantPlay = it.second;
					if ( true == (res & Playing::Change::CURRENT_TETRIMINO_MOVED) )
					{
						std::string curTet( participantPlay.currentTetriminoInfo() );
						packet.pack( TAG_MY_CURRENT_TETRIMINO, curTet );
					}
					if ( true == (res & Playing::Change::CURRENT_TETRIMINO_LANDED) )
					{
						std::string stage( participantPlay.stageInfo() );
						packet.pack( TAG_MY_STAGE, stage );
					}

					const ClientIndex participantIdx = it.first;
					Socket& participantSocket = clientS[ participantIdx ].socket( );
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
			}
			break;
		default:
			break;
	}
	return retVal;
}

void Room::start( )
{
	mState = State::STARTED;
}

void Room::kick( const ClientIndex index )
{
	mParticipantS.erase( index );
}

ClientIndex Room::hostIndex( ) const
{
	return mHostIndex;
}

bool Room::hasElapsedMs( const uint32_t milliseconds ) const
{
	return ( std::chrono::milliseconds(3000) < (Clock::now()-mStartTime) )? true: false;
}