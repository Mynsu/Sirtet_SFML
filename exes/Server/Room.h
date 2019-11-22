#pragma once
#include "Playing.h"

class Client;

using RoomID = int32_t;

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

	// Needs initialization.
	Room( );
	explicit Room( const ClientIndex hostIndex );
	Room( const Room& ) = delete;
	void operator=( const Room& ) = delete;
	~Room( ) = default;

	// NOTE: Called only in Server.cpp.  Not called in Client.cpp.
	std::forward_list<ClientIndex> update( std::vector<Client>& clientS );
	void start( );
	void kick( const ClientIndex index );
	ClientIndex hostIndex( ) const;
private:
	inline void restartTimer( )
	{
		mStartTime = Clock::now( );
	}
	bool hasElapsedMs( const uint32_t milliseconds ) const;
	ClientIndex mHostIndex;
	Room::State mState;
	Clock::time_point mStartTime;
	std::unordered_map< ClientIndex, Playing > mParticipantS;
};