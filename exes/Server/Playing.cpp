#include "pch.h"
#include "Playing.h"

const uint8_t FALLING_DOWN_INTERVAL_MS = 17;
const uint8_t FALLING_DOWN_SPEED = 3;
const uint32_t ASYNC_TOLERANCE_MS = 2500;
const uint32_t GAME_OVER_CHK_INTERVAL_MS = 250;

Playing::Playing()
	: mHasTetriminoCollidedOnClient( false ),
	mHasTetriminoCollidedOnServer( false ),
	mIsGameOver_( false ),
	mNumOfLinesCleared( 0 ), mTempoMs( 1000 ),
	mMoveToUpdate( ::model::tetrimino::Move::NONE_MAX ),
	mAlarms{ Clock::now() }
{	
	for ( auto& it : mAlarms )
	{
		it = Clock::now( );
	}
}

void Playing::spawnTetrimino()
{
	mCurrentTetrimino = ::model::Tetrimino::Spawn();
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
} 

void Playing::perceive( const ::model::tetrimino::Move move )
{
	ASSERT_TRUE( move < ::model::tetrimino::Move::NONE_MAX );
	mMoveToUpdate = move;
}

void Playing::perceive( const bool hasTetriminoCollidedOnClient )
{
	mHasTetriminoCollidedOnClient = hasTetriminoCollidedOnClient;
}

bool Playing::update( )
{
	bool isAsyncTolerable = true;
	mUpdateResult = Playing::UpdateResult::NONE;
	if ( true == mIsGameOver_ )
	{
		return isAsyncTolerable;
	}

	if ( true == mHasTetriminoCollidedOnServer )
	{
		if ( true == mHasTetriminoCollidedOnClient )
		{
			mHasTetriminoCollidedOnClient = false;
			mHasTetriminoCollidedOnServer = false;
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

	bool hasCollidedOnServer = false;
	if ( true == mCurrentTetrimino.isFallingDown() )
	{
		if ( true == alarmAfter(FALLING_DOWN_INTERVAL_MS, AlarmIndex::TETRIMINO_DOWN) )
		{
			resetAlarm( AlarmIndex::TETRIMINO_DOWN );
			//TODO: 시간이 많이 지난 만큼 더 이동할까?
			for ( uint8_t i = 0; FALLING_DOWN_SPEED != i; ++i )
			{
				hasCollidedOnServer = mCurrentTetrimino.moveDown( mStage.cgrid() );
				if ( true == hasCollidedOnServer )
				{
					mCurrentTetrimino.fallDown( false );
					goto last;
				}
			}
			mUpdateResult = Playing::UpdateResult::TETRIMINO_MOVED;
			return true;
		}
	}
	else
	{
		switch ( mMoveToUpdate )
		{
			case ::model::tetrimino::Move::FALL_DOWN:
				mCurrentTetrimino.fallDown( );
				[[ fallthrough ]];
			case ::model::tetrimino::Move::DOWN:
				hasCollidedOnServer = mCurrentTetrimino.moveDown( mStage.cgrid() );
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
				std::cerr << "Undefined move.\n";
#ifdef _DEBUG
				__debugbreak( );
#else
				__assume( 0 );
#endif
				break;
		}
		if ( true == alarmAfter(mTempoMs, AlarmIndex::TETRIMINO_DOWN) )
		{
			//TODO: 시간이 많이 지난 만큼 더 이동할까?
			resetAlarm( AlarmIndex::TETRIMINO_DOWN );
			hasCollidedOnServer = mCurrentTetrimino.moveDown( mStage.cgrid() );
			mUpdateResult = Playing::UpdateResult::TETRIMINO_MOVED;
		}

		mMoveToUpdate = ::model::tetrimino::Move::NONE_MAX;
	}

	last:
	if ( true == hasCollidedOnServer )
	{
		mCurrentTetrimino.land( mStage.grid() );
		reloadTetrimino( );
		mUpdateResult = Playing::UpdateResult::TETRIMINO_LANDED;

		const uint8_t numOfLinesCleared = mStage.tryClearRow();
		if ( 0 != numOfLinesCleared )
		{
			mNumOfLinesCleared = numOfLinesCleared;
			mUpdateResult = Playing::UpdateResult::LINE_CLEARED;
		}
	}

	if ( true == mStage.isOver() )
	{
#ifdef _DEBUG
		std::cout << "Game is over.\n";
#endif
		mStage.blackout( );
		mUpdateResult = Playing::UpdateResult::GAME_OVER;
		mIsGameOver_ = true;
	}

	if ( Playing::UpdateResult::LINE_CLEARED == mUpdateResult ||
		Playing::UpdateResult::TETRIMINO_LANDED == mUpdateResult )
	{
		if ( false == mHasTetriminoCollidedOnClient )
		{
			mHasTetriminoCollidedOnServer = true;
			mAlarms[(int)AlarmIndex::ASYNC_TOLERANCE] = Clock::now();
		}
		else
		{
			mHasTetriminoCollidedOnClient = false;
		}
	}

	return true;
}

Playing::UpdateResult Playing::updateResult( ) const
{
	return mUpdateResult;
}

::model::tetrimino::Type Playing::currentTetriminoType() const
{
	return mCurrentTetrimino.type();
}

::model::tetrimino::Rotation Playing::currentTetriminoRotationID() const
{
	return mCurrentTetrimino.rotationID();
}

sf::Vector2<int8_t> Playing::currentTetriminoPosition() const
{
	return mCurrentTetrimino.position();
}

::model::tetrimino::Type Playing::nextTetriminoType() const
{
	return mNextTetriminos.front().type();
}

uint32_t Playing::tempoMs() const
{
	return mTempoMs;
}

void Playing::setRelativeTempoMs( const int32_t milliseconds )
{
	mTempoMs += milliseconds;
}

std::string Playing::serializedStage( ) const
{
	const ::model::stage::Grid& grid = mStage.cgrid();
	return std::string( (char*)&grid, sizeof(::model::stage::Grid) );
}

uint8_t Playing::numOfLinesCleared() const
{
	return mNumOfLinesCleared;
}

bool Playing::isGameOver() const
{
	return mIsGameOver_;
}
