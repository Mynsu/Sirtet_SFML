#include "pch.h"
#include "Playing.h"

const uint32_t FPS = 60;
const uint8_t MS_PER_FRAME = 1000/FPS;
const uint8_t FALLING_DIFF = 3;
const uint8_t TEMPO_DIFF_MS = 20;
const uint32_t ASYNC_TOLERANCE_MS = 1000;
const uint32_t GAME_OVER_CHK_INTERVAL_MS = 250;

Playing::Playing()
	: mHasTetriminoCollidedOnClient( false ),
	mIsWaitingUntilTetriminoCollidedOnClient( false ),
	mNumOfLinesCleared( 0 ), mTempoMs( 1000 ),
	mMoveToUpdate( ::model::tetrimino::Move::NONE_MAX )
{
}

void Playing::spawnTetrimino()
{
	mCurrentTetrimino = ::model::Tetrimino::Spawn();
	mNextTetriminoS.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminoS.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminoS.emplace( ::model::Tetrimino::Spawn() );
} 

void Playing::perceive( const ::model::tetrimino::Move move )
{
	ASSERT_TRUE( move < ::model::tetrimino::Move::NONE_MAX );
	mMoveToUpdate = move;
}

bool Playing::update( )
{
	bool isAsyncTolerable = true;
	mUpdateResult = Playing::UpdateResult::NONE;
	if ( true == mStage.isOver() )
	{
		return isAsyncTolerable;
	}

	if ( true == mIsWaitingUntilTetriminoCollidedOnClient )
	{
		if ( true == mHasTetriminoCollidedOnClient )
		{
			mHasTetriminoCollidedOnClient = false;
			mIsWaitingUntilTetriminoCollidedOnClient = false;
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

	bool hasCollided = false;
	if ( true == mCurrentTetrimino.isFallingDown() )
	{
		if ( true == alarmAfter(MS_PER_FRAME, AlarmIndex::TETRIMINO_FALLDOWN) )
		{
			// TODO: delta 줄여가면서 log로.
			for ( uint8_t i = 0; FALLING_DIFF != i; ++i )
			{
				hasCollided = mCurrentTetrimino.moveDown( mStage.cgrid() );
				if ( true == hasCollided )
				{
					mCurrentTetrimino.fallDown( false );
					goto last;
				}
			}
			return true;
		}
	}
	else
	{
		if ( true == alarmAfter(mTempoMs, AlarmIndex::TETRIMINO_FALLDOWN) )
		{
			hasCollided = mCurrentTetrimino.moveDown( mStage.cgrid() );
		}

		switch ( mMoveToUpdate )
		{
			case ::model::tetrimino::Move::FALL_DOWN:
				mCurrentTetrimino.fallDown( );
				[[ fallthrough ]];
			case ::model::tetrimino::Move::DOWN:
				hasCollided = mCurrentTetrimino.moveDown( mStage.cgrid() );
				mPast[(int)AlarmIndex::TETRIMINO_FALLDOWN] = Clock::now( );
				break;
			case ::model::tetrimino::Move::LEFT:
				mCurrentTetrimino.tryMoveLeft( mStage.cgrid() );
				break;
			case ::model::tetrimino::Move::RIGHT:
				mCurrentTetrimino.tryMoveRight( mStage.cgrid() );
				break;
			case ::model::tetrimino::Move::ROTATE:
				mCurrentTetrimino.tryRotate( mStage.cgrid() );
				break;
			case ::model::tetrimino::Move::NONE_MAX:
				break;
			default:
				std::cerr << "Undefined move.\n";
#ifdef _DEBUG
				__debugbreak( );
#else
				__assume( 0 );
#endif
				break;
		}
		mMoveToUpdate = ::model::tetrimino::Move::NONE_MAX;
	}

	last:
	if ( true == hasCollided )
	{
		mCurrentTetrimino.land( mStage.grid() );
		reloadTetrimino( );
		mUpdateResult = Playing::UpdateResult::TETRIMINO_LANDED;

		const uint8_t numOfLinesCleared = mStage.tryClearRow( );
		if ( 0 != numOfLinesCleared )
		{
			mNumOfLinesCleared = numOfLinesCleared;
			mTempoMs -= TEMPO_DIFF_MS;
			mUpdateResult = Playing::UpdateResult::LINE_CLEARED;
		}
	}

	if ( true == alarmAfter(GAME_OVER_CHK_INTERVAL_MS, AlarmIndex::GAME_OVER) 
		&& true == mStage.isOver() )
	{
		mStage.blackout( );
		mUpdateResult = Playing::UpdateResult::GAME_OVER;
	}

	if ( Playing::UpdateResult::LINE_CLEARED == mUpdateResult ||
		Playing::UpdateResult::TETRIMINO_LANDED == mUpdateResult )
	{
		mIsWaitingUntilTetriminoCollidedOnClient = true;
		mPast[(int)AlarmIndex::ASYNC_TOLERANCE] = Clock::now();
	}

	return true;
}

Playing::UpdateResult Playing::updateResult( )
{
	return mUpdateResult;
}

std::string Playing::tetriminoOnNet( )
{
	::model::tetrimino::Type tetriminoOnNet =
		(::model::tetrimino::Type)::htonl( (u_long)mCurrentTetrimino.type() );
	return std::string( (char*)&tetriminoOnNet, sizeof(tetriminoOnNet) );
}

std::string Playing::tempoMsOnNet()
{
	const uint32_t tempoMsOnNet = ::htonl(mTempoMs);
	return std::string( (char*)&tempoMsOnNet, sizeof(tempoMsOnNet) );
}

std::string Playing::stageOnNet( )
{
	::model::stage::Grid& grid = mStage.grid( );
	return std::string( (char*)&grid, sizeof(::model::stage::Grid) );
}

void Playing::perceive( const bool hasTetriminoCollidedInClient )
{
	mHasTetriminoCollidedOnClient = hasTetriminoCollidedInClient;
}

uint8_t Playing::numOfLinesCleared() const
{
	return mNumOfLinesCleared;
}
