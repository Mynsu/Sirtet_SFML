#include "pch.h"
#include "Playing.h"

const uint32_t FPS = 60;
constexpr uint8_t MS_PER_FRAME = 1000/FPS;
const uint32_t ASYNC_TOLERANCE_MS = 1000;

Playing::Playing()
	: mTempoMs( 1000 ), mIsAsync( false )
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
	bool retVal = true;
	mUpdateResult = Playing::UpdateResult::NONE;
	if ( true == mIsAsync )
	{
		mMoveToUpdate = ::model::tetrimino::Move::NONE_MAX;
		// Exception
		if ( true == alarmAfter(ASYNC_TOLERANCE_MS) )
		{
			retVal = false;
		}
		return retVal;
	}

	bool hasCollided = false;
	if ( true == mCurrentTetrimino.isFallingDown() )
	{
		if ( true == alarmAfter(MS_PER_FRAME) )
		{
			// TODO: delta 줄여가면서 log로.
			hasCollided = mCurrentTetrimino.moveDown( mStage.cgrid() );
			if ( true == hasCollided )
			{
				mCurrentTetrimino.fallDown( false );
			}
		}
	}
	else
	{
		if ( true == alarmAfter(mTempoMs) )
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
				mOldTime = Clock::now( );
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
			default:
				break;
		}
		mMoveToUpdate = ::model::tetrimino::Move::NONE_MAX;
	}

	if ( true == hasCollided )
	{
		mCurrentTetrimino.land( mStage.grid() );
		reloadTetrimino( );
		mTempoMs -= 20;
		mUpdateResult = Playing::UpdateResult::TETRIMINO_LANDED;
	}
	return retVal;
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
	const uint32_t tempoMsOnNet = (uint32_t)::htonl((u_long)mTempoMs);
	return std::string( (char*)&tempoMsOnNet, sizeof(tempoMsOnNet) );
}

std::string Playing::stageOnNet( )
{
	::model::stage::Grid& grid = mStage.grid( );
	return std::string( (char*)&grid, sizeof(::model::stage::Grid) );
}

void Playing::synchronize( const bool async )
{
	if ( async != mIsAsync )
	{
		mIsAsync = async;
		mOldTime = Clock::now( );
	}
}
