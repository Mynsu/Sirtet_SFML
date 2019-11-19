#pragma once

class Room
{
public:
	enum class State
	{
		DOING_NOTHING,
		READY,
		PLAYING,
	};

	Room( );
	explicit Room( const Index hostIndex );
	Room( const Room& ) = delete;
	void operator=( const Room& ) = delete;
	~Room( ) = default;

	void restartTimer( );
	State state( ) const;
	void setState( State state );
	bool hasElapsedMs( const uint32_t duration ) const;
	void join( const Index index );
	Index hostIndex( ) const;
	std::unordered_set< Index > mParticipantS;
private:
	Index mHostIndex;
	State mState;
	Clock::time_point mStartTime;
};