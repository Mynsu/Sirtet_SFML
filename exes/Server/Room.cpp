#include "pch.h"
#include "Room.h"

Room::Room( )
	: mHostIndex( -1 ), mState( State::DOING_NOTHING )
{
}

Room::Room( const Index hostIndex )
	: mHostIndex( hostIndex ), mState( State::DOING_NOTHING )
{
	mParticipantS.emplace( hostIndex );
}

void Room::restartTimer( )
{
	mStartTime = Clock::now( );
}

Room::State Room::state( ) const
{
	return mState;
}

void Room::setState( State state )
{
	mState = state;
}

bool Room::hasElapsedMs( const uint32_t duration ) const
{
	return (std::chrono::milliseconds( duration )<(Clock::now( )-mStartTime))? true: false;
}

void Room::join( const Index index )
{
	mParticipantS.emplace( index );
}

Index Room::hostIndex( ) const
{
	return mHostIndex;
}
