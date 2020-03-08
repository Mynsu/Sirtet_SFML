#pragma once
#include "Playing.h"

class Client;

class Room
{
public:
	enum class State
	{
		WAITING,
		ON_START,
		READY,
		PLAYING,
		ALL_OVER,
	};
	enum class Perception
	{
		INSTANTIATION_DONE,
		INITIATION_DONE,
		TETRIMINO_LAND_ON_CLIENT,
	};

	explicit Room( const ClientIndex hostIndex );
	Room( const Room& ) = delete;
	Room( Room&& ) = delete;
	void operator=( const Room& ) = delete;
	virtual ~Room( ) = default;

	void start( )
	{
		mState = State::ON_START;
	}
	// Kicks the client out.
	// Returns 0 if no one remains, otherwise returns 1.
	int8_t pop( const ClientIndex index );
	void perceive( const ClientIndex index, const ::model::tetrimino::Move move )
	{
		if ( auto it = mPlayingParticipants.find(index); mPlayingParticipants.end() != it )
		{
			it->second.moveTetrimino( move );
		}
		// Exception
		else
		{
			std::cerr << "Client " << index << " knows the incorrect room ID.\n";
		}
	}
	void perceive( const ClientIndex index, const Perception perceptedThing );
	std::vector<ClientIndex> update( std::array<Client, CLIENT_CAPACITY>& clients );
	std::vector<ClientIndex> notify( std::array<Client, CLIENT_CAPACITY>& clients );
	ClientIndex hostIndex( ) const
	{
		return mHostIndex;
	}
	bool tryEmplace( const ClientIndex index );
private:
	enum class AlarmIndex
	{
		START,
		UPDATE_USER_LIST,
		NONE_MAX,
	};
	struct Observer
	{
	public:
		enum class State
		{
			ZERO,
			INSTANTIATION_DONE,
			DONE,
		};
		Observer( const ClientIndex _index, const bool hasJoinedWhilePlaying )
			: index( _index )
		{
			if ( true == hasJoinedWhilePlaying )
			{
				state = State::ZERO;
			}
			else
			{
				state = State::DONE;
			}
		}
		ClientIndex index;
		State state;
	};
	bool alarmAfterAndReset( const uint16_t milliseconds, const AlarmIndex index )
	{
		bool elapsed = false;
		const Clock::time_point now = Clock::now();
		if ( std::chrono::milliseconds(milliseconds) < now-mAlarms[(int)index] )
		{
			mAlarms[(int)index] = now;
			elapsed = true;
		}
		return elapsed;
	}
	static const uint8_t PARTICIPANT_CAPACITY = 4;
	bool mHasTempoChanged_, mHasHostChanged_;
	ClientIndex mHostIndex;
	Room::State mState;
	std::vector<Observer> mObservers;
	Clock::time_point mAlarms[(int)AlarmIndex::NONE_MAX];
	std::unordered_map<ClientIndex, Playing> mPlayingParticipants;
};