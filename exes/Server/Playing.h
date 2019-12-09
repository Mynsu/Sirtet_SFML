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
	};

	Playing( );
	Playing( const Playing& ) = default;
	void operator=( const Playing& ) = delete;
	~Playing( ) = default;

	void spawnTetrimino( );
	void perceive( const ::model::tetrimino::Move move );
	bool update( );
	Playing::UpdateResult updateResult( );
	std::string tetriminoOnNet( );
	std::string tempoMsOnNet( );
	std::string stageOnNet( );
	void synchronize( const bool async );
private:
	inline void reloadTetrimino( )
	{
		mCurrentTetrimino = mNextTetriminoS.front( );
		mNextTetriminoS.pop( );
		mNextTetriminoS.emplace( ::model::Tetrimino::Spawn() );
	}
	inline bool alarmAfter( const uint32_t milliseconds )
	{
		const Clock::time_point now = Clock::now( );
		if ( std::chrono::milliseconds(milliseconds) < (now-mOldTime) )
		{
			mOldTime = now;
			return true;
		}
		else
		{
			return false;
		}
	}
	bool mIsAsync;
	Clock::time_point mOldTime;
	uint32_t mTempoMs;
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