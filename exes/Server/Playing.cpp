#include "pch.h"
#include "Playing.h"

const uint8_t FALLING_DIFF = 3u;

Playing::Playing()
	: mTick( std::chrono::milliseconds(1000) )
{
}

void Playing::spawnTetrimino()
{
	mCurrentTetrimino = ::model::Tetrimino::Spawn( );
	mNextTetriminoS.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminoS.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminoS.emplace( ::model::Tetrimino::Spawn() );
} 

Playing::Change Playing::update( )
{
	Playing::Change retVal = Playing::Change::NONE;
	const Clock::time_point now = Clock::now( );
	if ( mTick < now-mOld )
	{
		mOld = now;
		for ( uint8_t i = 0u; FALLING_DIFF != i; ++i )
		{
			if ( true == mCurrentTetrimino.moveDown(mStage.grid()) )
			{
				mCurrentTetrimino.land( mStage.floor() );
				reloadTetrimino( );
				mTick -= std::chrono::milliseconds(20);
				retVal |= Playing::Change::CURRENT_TETRIMINO_LANDED;
				break;
			}
			else
			{
				if ( false == (retVal & Playing::Change::CURRENT_TETRIMINO_MOVED) )
				{
					retVal |= Playing::Change::CURRENT_TETRIMINO_MOVED;
				}
			}
		}
	}
	return retVal;
}

std::string Playing::currentTetriminoInfo( )
{
	///mOld = Clock::now( );
	::model::tetrimino::Info info;
	info.position = mCurrentTetrimino.position( );
	info.type = (decltype(info.type))::htonl( (u_long)mCurrentTetrimino.type() );
	return std::string( (char*)&info, sizeof(::model::tetrimino::Info) );
}

std::string Playing::gridInfo()
{

}