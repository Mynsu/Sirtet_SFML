#pragma once
#include "Tetrimino.h"
#include "Stage.h"

class Playing
{
public:
	enum class UpdateResult
	{
		NONE,
		TETRIMINO_MOVED,
		TETRIMINO_LANDED,
		LINE_CLEARED,
		GAME_OVER,
	};

	Playing( );
	Playing( const Playing& ) = delete;
	Playing( Playing&& ) = delete;
	void operator=( const Playing& ) = delete;
	virtual ~Playing( ) = default;

	// Returns true if the time gap between the server and the client,
	// otherwise returns false.
	bool update( );
	Playing::UpdateResult updateResultToNotify( ) const
	{
		return mUpdateResult;
	}
	void spawnTetriminos( )
	{
		mCurrentTetrimino = ::model::Tetrimino::Spawn();
		mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
		mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
		mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	}
	void moveTetrimino( const ::model::tetrimino::Move move )
	{
		ASSERT_TRUE( move < ::model::tetrimino::Move::NONE_MAX );
		mMoveToUpdate = move;
	}
	void perceive( const bool hasTetriminoLandedOnClient = true )
	{
		mHasTetriminoLandedOnClient = hasTetriminoLandedOnClient;
	}
	::model::tetrimino::Type currentTetriminoType( ) const
	{
		return mCurrentTetrimino.type();
	}
	::model::tetrimino::Rotation currentTetriminoRotationID( ) const
	{
		return mCurrentTetrimino.rotationID();
	}
	sf::Vector2<int8_t> currentTetriminoPosition( ) const
	{
		return mCurrentTetrimino.position();
	}
	::model::tetrimino::Type nextTetriminoType( ) const
	{
		return mNextTetriminos.front().type();
	}
	uint16_t tempoMs( ) const
	{
		return mTempoMs;
	}
	void setRelativeTempoMs( const int32_t milliseconds )
	{
		mTempoMs += milliseconds;
	}
	const ::model::stage::Grid& serializedStage( ) const
	{
		return mStage.cgrid();
	}
	uint8_t numOfLinesClearedRecently( ) const
	{
		return mNumOfLinesClearedRecently;
	}
	bool isGameOver( ) const
	{
		return mIsGameOver_;
	}
private:
	enum class AlarmIndex
	{
		TETRIMINO_DOWN,
		ASYNC_TOLERANCE,
		GAME_OVER,
		NONE_MAX,
	};
	void reloadTetrimino( )
	{
		mCurrentTetrimino = mNextTetriminos.front();
		mNextTetriminos.pop( );
		mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	}
	// This function doesn't reset the alarm.
	bool alarmAfter( const uint16_t milliseconds, const AlarmIndex index )
	{
		bool elapsed = false;
		if ( std::chrono::milliseconds(milliseconds) < Clock::now()-mAlarms[(int)index] )
		{
			elapsed = true;
		}
		return elapsed;
	}
	void resetAlarm( const AlarmIndex index )
	{
		mAlarms[(int)index] = Clock::now();
	}
	bool mHasTetriminoLandedOnClient, mHasTetriminoLandedOnServer, mIsGameOver_;
	uint8_t mNumOfLinesClearedRecently;
	uint16_t mTempoMs;
	Clock::time_point mAlarms[(int)AlarmIndex::NONE_MAX];
	::model::tetrimino::Move mMoveToUpdate;
	UpdateResult mUpdateResult;
	::model::Tetrimino mCurrentTetrimino;
	std::queue<::model::Tetrimino> mNextTetriminos;
	::model::Stage mStage;
};