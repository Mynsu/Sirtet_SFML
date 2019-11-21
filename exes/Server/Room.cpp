#include "pch.h"
#include "Room.h"
#include "Client.h"

//Room::Room( )
//	: mHostIndex( -1 ), mState( State::DOING_NOTHING )
//{
//}

Room::Room( const Index hostIndex )
	: mHostIndex( hostIndex ), mState( State::DOING_NOTHING )
{
	mParticipantS.emplace( hostIndex );
}

std::forward_list<Index> Room::update( std::vector<Client>& clientS )
{
	std::forward_list<Index> retVal;
	switch ( mState )
	{
		case Room::State::DOING_NOTHING:
			break;
		case Room::State::STARTED:
			for ( const auto idx : mParticipantS )
			{
				Socket& participantSocket = clientS[ idx ].socket( );
				std::string req( TAG_GET_READY );
				if ( -1 == participantSocket.sendOverlapped(req.data(), req.size()) )
				{
					// Exception
					std::cerr << "Failed to notify Client " << idx
						<< " that the game get started.\n";
					retVal.emplace_front( idx );
				}
				participantSocket.pend( );
				restartTimer( );
				clientS[ idx ].setState( Client::State::READY );
			}
			mState = Room::State::READY;
			break;
		case Room::State::READY:
			if ( true == hasElapsedMs(3000) )
			{
#ifdef _DEBUG
				std::cout << "Room hosted by " << mHostIndex << " gets started now.\n";
#endif
				for ( const auto idx : mParticipantS )
				{
					Client& participant = clientS[ idx ];
					Socket& participantSocket = participant.socket( );
					Playing& playing = participant.playing( );
					std::string data( playing.currentTetriminoInfo() );
					std::string curTet( TAG_CURRENT_TETRIMINO + std::to_string(data.size())
									   + TOKEN_SEPARATOR_2
									   + data );
					if ( -1 ==	participantSocket.sendOverlapped(curTet.data(), curTet.size()) )
					{
						// Exception
						std::cerr << "Failed to notify Client " << idx
							<< " that the starting countdown ended.\n";
						retVal.emplace_front( idx );
					}
					participantSocket.pend( );
					participant.setState( Client::State::PLAYING );
				}
				restartTimer( );
				mState = Room::State::PLAYING;
			}
			break;
		case Room::State::PLAYING:

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

void Room::kick( const Index index )
{
	mParticipantS.erase( index );
}

Index Room::hostIndex( ) const
{
	return mHostIndex;
}

void Room::restartTimer()
{
	mStartTime = Clock::now( );
}

bool Room::hasElapsedMs( const uint32_t milliseconds ) const
{
	return ( std::chrono::milliseconds(3000) < (Clock::now()-mStartTime) )? true: false;
}
