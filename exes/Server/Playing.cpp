#include "pch.h"
#include "Playing.h"

const uint8_t HARD_DROP_DELTA_TIME_MS = 17;
const uint8_t HARD_DROP_SPEED = 3;
const uint16_t ASYNC_TOLERANCE_MS = 2500;
const uint16_t GAME_OVER_CHK_INTERVAL_MS = 250;

Playing::Playing( )
	: mHasTetriminoLandedOnClient( false ),
	mHasTetriminoLandedOnServer( false ),
	mIsGameOver_( false ),
	mNumOfLinesClearedRecently( 0 ), mTempoMs( 1000 ),
	mMoveToUpdate( ::model::tetrimino::Move::NONE_MAX )
{
	const Clock::time_point init = Clock::now();
	for ( Clock::time_point& alarm : mAlarms )
	{
		alarm = init;
	}
}

bool Playing::update( )
{
	bool isAsyncTolerable = true;
	mUpdateResult = Playing::UpdateResult::NONE;
	if ( true == mIsGameOver_ )
	{
		// Doing nothing.
		return isAsyncTolerable;
	}

	if ( true == mHasTetriminoLandedOnServer )
	{
		if ( true == mHasTetriminoLandedOnClient )
		{
			mHasTetriminoLandedOnClient = false;
			mHasTetriminoLandedOnServer = false;
		}
		else
		{
			mMoveToUpdate = ::model::tetrimino::Move::NONE_MAX;
			// Exception
			if ( true == alarmAfter(ASYNC_TOLERANCE_MS, AlarmIndex::ASYNC_TOLERANCE) )
			{
				isAsyncTolerable = false;
			}
			return isAsyncTolerable;
		}
	}

	bool hasTetriminoLandedOnServer = false;
	if ( true == mCurrentTetrimino.isHardDropping() )
	{
		if ( true == alarmAfter(HARD_DROP_DELTA_TIME_MS, AlarmIndex::TETRIMINO_DOWN) )
		{
			resetAlarm( AlarmIndex::TETRIMINO_DOWN );
			for ( uint8_t i = 0; HARD_DROP_SPEED != i; ++i )
			{
				hasTetriminoLandedOnServer = mCurrentTetrimino.moveDown(mStage.cgrid());
				if ( true == hasTetriminoLandedOnServer )
				{
					mCurrentTetrimino.hardDrop( false );
					break;
				}
			}
			if ( false == hasTetriminoLandedOnServer )
			{
				mUpdateResult = Playing::UpdateResult::TETRIMINO_MOVED;
				return isAsyncTolerable;
			}
		}
	}
	else
	{
		switch ( mMoveToUpdate )
		{
			case ::model::tetrimino::Move::HARD_DROP:
				mCurrentTetrimino.hardDrop( );
				[[ fallthrough ]];
			case ::model::tetrimino::Move::DOWN:
				hasTetriminoLandedOnServer = mCurrentTetrimino.moveDown(mStage.cgrid());
				mAlarms[(int)AlarmIndex::TETRIMINO_DOWN] = Clock::now();
				mUpdateResult = Playing::UpdateResult::TETRIMINO_MOVED;
				break;
			case ::model::tetrimino::Move::LEFT:
				mCurrentTetrimino.tryMoveLeft( mStage.cgrid() );
				mUpdateResult = Playing::UpdateResult::TETRIMINO_MOVED;
				break;
			case ::model::tetrimino::Move::RIGHT:
				mCurrentTetrimino.tryMoveRight( mStage.cgrid() );
				mUpdateResult = Playing::UpdateResult::TETRIMINO_MOVED;
				break;
			case ::model::tetrimino::Move::ROTATE:
				mCurrentTetrimino.tryRotate( mStage.cgrid() );
				mUpdateResult = Playing::UpdateResult::TETRIMINO_MOVED;
				break;
			case ::model::tetrimino::Move::NONE_MAX:
				break;
			default:
#ifdef _DEBUG
				__debugbreak( );
#else
				__assume( 0 );
#endif
		}
		if ( true == alarmAfter(mTempoMs, AlarmIndex::TETRIMINO_DOWN) )
		{
			resetAlarm( AlarmIndex::TETRIMINO_DOWN );
			hasTetriminoLandedOnServer = mCurrentTetrimino.moveDown(mStage.cgrid());
			mUpdateResult = Playing::UpdateResult::TETRIMINO_MOVED;
		}

		mMoveToUpdate = ::model::tetrimino::Move::NONE_MAX;
	}

	if ( true == hasTetriminoLandedOnServer )
	{
		mCurrentTetrimino.land( mStage.grid() );
		reloadTetrimino( );
		mUpdateResult = Playing::UpdateResult::TETRIMINO_LANDED;

		const uint8_t numOfLinesCleared = mStage.tryClearRow();
		if ( 0 != numOfLinesCleared )
		{
			mNumOfLinesClearedRecently = numOfLinesCleared;
			mUpdateResult = Playing::UpdateResult::LINE_CLEARED;
		}
	}

	if ( true == mStage.isOver() )
	{
#ifdef _DEV
		std::cout << "Game is over.\n";
#endif
		mStage.blackout( );
		mUpdateResult = Playing::UpdateResult::GAME_OVER;
		mIsGameOver_ = true;
	}

	if ( Playing::UpdateResult::LINE_CLEARED == mUpdateResult ||
		Playing::UpdateResult::TETRIMINO_LANDED == mUpdateResult )
	{
		if ( false == mHasTetriminoLandedOnClient )
		{
			mHasTetriminoLandedOnServer = true;
			mAlarms[(int)AlarmIndex::ASYNC_TOLERANCE] = Clock::now();
		}
		else
		{
			mHasTetriminoLandedOnClient = false;
		}
	}

	return isAsyncTolerable;
}