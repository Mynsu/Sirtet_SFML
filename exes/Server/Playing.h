#pragma once
#include "Tetrimino.h"

class Playing
{
public:
	Playing( ) = default;
	Playing( const Playing& ) = default;
	void operator=( const Playing& ) = delete;
	~Playing( ) = default;

	void spawnTetrimino( );
	std::string currentTetriminoInfo( );
private:
	::model::Tetrimino mCurrentTetrimino;
};