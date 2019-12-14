#pragma once
#include "Playing.h"

class Client;

const uint8_t PARTICIPANT_CAPACITY = 4u;

class Room
{
public:
	enum class State
	{
		WAITING,
		STARTED,
		READY,
		PLAYING,
	};

	// !IMPORTANT: DO NOT USE!  Defined to use std::unordered_map.
	Room( );
	explicit Room( const ClientIndex hostIndex );
	Room( const Room& ) = delete;
	void operator=( const Room& ) = delete;
	~Room( ) = default;

	void start( );
	bool leave( const ClientIndex index );
	void perceive( const ClientIndex index, const ::model::tetrimino::Move move );
	void perceive( const ClientIndex index, const bool hasTetriminoCollidedInClient = true );
	std::forward_list<ClientIndex> update( );
	std::forward_list<ClientIndex> notify( std::vector<Client>& clientS );
	// MUST destruct this room when returning false, which means having kicked out the last one.
	ClientIndex hostIndex( ) const;
private:
	inline bool alarmAfter( const uint32_t milliseconds )
	{
		if ( std::chrono::milliseconds(milliseconds) < (Clock::now()-mStartTime) )
		{
			mStartTime = Clock::now( );
			return true;
		}
		else
		{
			return false;
		}
	}
	ClientIndex mHostIndex;
	Room::State mState;
	Clock::time_point mStartTime;
	std::unordered_map< ClientIndex, Playing > mGuestS;
};