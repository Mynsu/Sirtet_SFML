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
	bool tryAccept( const ClientIndex index );
private:
	enum class AlarmIndex
	{
		START,
		UPDATE_USER_LIST,
		NONE_MAX,
	};
	inline bool alarmAfter( const uint32_t milliseconds, const AlarmIndex index )
	{
		const Clock::time_point now = Clock::now();
		if ( std::chrono::milliseconds(milliseconds) < (now-mAlarms[(int)index]) )
		{
			mAlarms[(int)index] = now;
			return true;
		}
		else
		{
			return false;
		}
	}
	ClientIndex mHostIndex;
	Room::State mState;
	std::vector< ClientIndex > mCandidateGuestS;
	Clock::time_point mAlarms[(int)AlarmIndex::NONE_MAX];
	std::unordered_map< ClientIndex, Playing > mGuestS;
};