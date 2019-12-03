#include "pch.h"
#include "Playing.h"

const uint8_t FALLING_DIFF = 3u;

Playing::Playing()
	: mTempo( std::chrono::milliseconds(1000) )
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
	if ( mTempo < now-mOldTime )
	{
		mOldTime = now;
		for ( uint8_t i = 0u; FALLING_DIFF != i; ++i )
		{
			if ( true == mCurrentTetrimino.moveDown(mStage.cgrid()) )
			{
				mCurrentTetrimino.land( mStage.grid() );
				reloadTetrimino( );
				mTempo -= std::chrono::milliseconds(20);
				retVal |= Playing::Change::CURRENT_TETRIMINO_LANDED;
				break;
			}
			else
			{
				if ( !(retVal & Playing::Change::CURRENT_TETRIMINO_MOVED) )
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
	::model::tetrimino::Info info;
	info.position = mCurrentTetrimino.position( );
	info.type = (decltype(info.type))::htonl( (u_long)mCurrentTetrimino.type() );
	return std::string( (char*)&info, sizeof(::model::tetrimino::Info) );
}

std::string Playing::stageInfo()
{
	::model::stage::Grid& grid = mStage.grid( );
	return std::string( (char*)&grid, sizeof(::model::stage::Grid) );
}