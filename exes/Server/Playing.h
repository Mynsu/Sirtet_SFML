#pragma once
#include "Tetrimino.h"
#include "Stage.h"

class Playing
{
public:
	enum class Change
	{
		NONE = 0b0,
		CURRENT_TETRIMINO_MOVED = 0b1,
		CURRENT_TETRIMINO_LANDED = 0b1<<1,
	};

	Playing( );
	Playing( const Playing& ) = default;
	void operator=( const Playing& ) = delete;
	~Playing( ) = default;

	void spawnTetrimino( );
	Playing::Change update( );
	std::string currentTetriminoInfo( );
	std::string stageInfo( );
private:
	inline void reloadTetrimino( )
	{
		mCurrentTetrimino = mNextTetriminoS.front( );
		mNextTetriminoS.pop( );
		mNextTetriminoS.emplace( ::model::Tetrimino::Spawn() );
	}
	Clock::time_point mOldTime;
	Clock::duration mTempo;
	::model::Tetrimino mCurrentTetrimino;
	std::queue< ::model::Tetrimino > mNextTetriminoS;
	::model::Stage mStage;
};

inline Playing::Change operator|( const Playing::Change lh, const Playing::Change rh )
{
	return (Playing::Change)((uint32_t)lh | (uint32_t)rh);
}

inline Playing::Change& operator|=( Playing::Change& lh, const Playing::Change rh )
{
	return lh = lh | rh;
}

inline bool operator&( const Playing::Change lh, const Playing::Change rh )
{
	return (bool)((uint32_t)lh & (uint32_t)rh);
}