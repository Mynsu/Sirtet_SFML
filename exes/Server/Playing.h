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
	void operator=( const Playing& ) = delete;
	virtual ~Playing( ) = default;

	void spawnTetrimino( );
	void perceive( const ::model::tetrimino::Move move );
	void perceive( const bool hasTetriminoLandedInClient = true );
	bool update( );
	Playing::UpdateResult updateResult( ) const;
	::model::tetrimino::Type currentTetriminoType( ) const;
	::model::tetrimino::Rotation currentTetriminoRotationID( ) const;
	sf::Vector2<int8_t> currentTetriminoPosition( ) const;
	::model::tetrimino::Type nextTetriminoType( ) const;
	uint32_t tempoMs( ) const;
	void setRelativeTempoMs( const int32_t milliseconds );
	std::string serializedStage( ) const;
	uint8_t numOfLinesCleared( ) const;
	bool isGameOver() const;
private:
	void reloadTetrimino( )
	{
		mCurrentTetrimino = mNextTetriminos.front( );
		mNextTetriminos.pop( );
		mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	}
	enum class AlarmIndex
	{
		TETRIMINO_DOWN,
		ASYNC_TOLERANCE,
		GAME_OVER,
		NONE_MAX,
	};
	// This function doesn't reset the alarm.
	bool alarmAfter( const uint32_t milliseconds, const AlarmIndex index )
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
		mAlarms[(int)index] = Clock::now( );
	}
	bool mHasTetriminoLandedOnClient, mHasTetriminoLandedOnServer, mIsGameOver_;
	uint8_t mNumOfLinesCleared;
	uint32_t mTempoMs;
	Clock::time_point mAlarms[ (int)AlarmIndex::NONE_MAX ];
	::model::tetrimino::Move mMoveToUpdate;
	UpdateResult mUpdateResult;
	::model::Tetrimino mCurrentTetrimino;
	std::queue<::model::Tetrimino> mNextTetriminos;
	::model::Stage mStage;
};