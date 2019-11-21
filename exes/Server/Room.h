#pragma once

class Client;

class Room
{
public:
	enum class State
	{
		DOING_NOTHING,
		STARTED,
		READY,
		PLAYING,
	};

	// Needs initialization.
	Room( ) = default;
	explicit Room( const Index hostIndex );
	Room( const Room& ) = delete;
	void operator=( const Room& ) = delete;
	~Room( ) = default;

	std::forward_list<Index> update( std::vector<Client>& clientS );
	void start( );
	void kick( const Index index );
	Index hostIndex( ) const;
private:
	void restartTimer( );
	bool hasElapsedMs( const uint32_t milliseconds ) const;
	Index mHostIndex;
	State mState;
	Clock::time_point mStartTime;
	std::unordered_set< Index > mParticipantS;
};