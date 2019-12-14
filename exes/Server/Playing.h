#pragma once
#include "Tetrimino.h"
#include "Stage.h"

class Playing
{
public:
	enum class UpdateResult
	{
		NONE,
		TETRIMINO_LANDED,
		LINE_CLEARED,
		GAME_OVER,
	};

	Playing( );
	Playing( const Playing& ) = default;
	void operator=( const Playing& ) = delete;
	~Playing( ) = default;

	void spawnTetrimino( );
	void perceive( const ::model::tetrimino::Move move );
	bool update( );
	Playing::UpdateResult updateResult( );
	::model::tetrimino::Type currentTetriminoType( ) const;
	::model::tetrimino::Type nextTetriminoType( ) const;
	uint32_t tempoMs( ) const;
	std::string stageOnNet( );
	void perceive( const bool hasTetriminoCollidedInClient = true );
	uint8_t numOfLinesCleared( ) const;
private:
	inline void reloadTetrimino( )
	{
		mCurrentTetrimino = mNextTetriminoS.front( );
		mNextTetriminoS.pop( );
		mNextTetriminoS.emplace( ::model::Tetrimino::Spawn() );
	}
	enum class AlarmIndex
	{
		TETRIMINO_FALLDOWN,
		ASYNC_TOLERANCE,
		GAME_OVER,
		NONE_MAX,
	};
	inline bool alarmAfter( const uint32_t milliseconds, const AlarmIndex index )
	{
		if ( std::chrono::milliseconds(milliseconds) < Clock::now()-mPast[(int)index] )
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	inline void resetAlarm( const AlarmIndex index )
	{
		mPast[(int)index] = Clock::now( );
	}
	bool mHasTetriminoCollidedOnClient, mIsWaitingUntilTetriminoCollidedOnClient;
	uint8_t mNumOfLinesCleared;
	uint32_t mTempoMs;
	Clock::time_point mPast[ (int)AlarmIndex::NONE_MAX ];
	::model::tetrimino::Move mMoveToUpdate;
	UpdateResult mUpdateResult;
	::model::Tetrimino mCurrentTetrimino;
	std::queue< ::model::Tetrimino > mNextTetriminoS;
	::model::Stage mStage;
};

inline Playing::UpdateResult operator|( const Playing::UpdateResult lh, const Playing::UpdateResult rh )
{
	return (Playing::UpdateResult)((uint32_t)lh | (uint32_t)rh);
}

inline Playing::UpdateResult& operator|=( Playing::UpdateResult& lh, const Playing::UpdateResult rh )
{
	return lh = lh | rh;
}

inline bool operator&( const Playing::UpdateResult lh, const Playing::UpdateResult rh )
{
	return (bool)((uint32_t)lh & (uint32_t)rh);
}