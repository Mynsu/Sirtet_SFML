#include "pch.h"
#include "Playing.h"

void Playing::spawnTetrimino()
{
	mCurrentTetrimino = ::model::Tetrimino::Spawn( );
} 

std::string Playing::currentTetriminoInfo( )
{
	::model::tetrimino::Info info;
	info.position = mCurrentTetrimino.position( );
	info.type = (decltype(info.type))::htonl( (u_long)mCurrentTetrimino.type() );
	///info.rotationID = (decltype(info.rotationID))::htonl( (u_long)mCurrentTetrimino.rotationID() );
	return std::string( (char*)&info, sizeof(::model::tetrimino::Info) );
}
