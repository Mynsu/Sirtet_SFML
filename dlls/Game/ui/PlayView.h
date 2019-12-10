#pragma once
#include "../model/Tetrimino.h"

namespace scene::online
{
	class Online;
}

namespace ui
{
	class PlayView
	{
	public:
		// NOTE: DO NOT USE.  Declared to use std::unordered_map.
		PlayView( );
		PlayView( sf::RenderWindow& window, ::scene::online::Online& net );
		~PlayView( ) = default;
		
		void setDimension( const sf::Vector2f position, const float cellSize );
		bool update( std::list<sf::Event>& eventQueue );
		void draw( const int time );
	private:
		using Clock = std::chrono::high_resolution_clock;
		bool mHasTetrimino, mHasTetriminoCollidedInClient, mIsGameOver;
		uint8_t mNumOfLinesCleared;
		// NOTE: Also as mHasTetriminoCollidedIn-Server.
		uint32_t mFrameCount_sync;
		uint32_t mFrameCount_input, mFrameCount_clearingVFX;
		float mCellSize;
		Clock::time_point mPast_falldown;
		Clock::duration	mTempoMs;
		sf::RenderWindow* mWindow_;
		::scene::online::Online* mNet;
		sf::Texture mTexture;
		sf::Sprite mSprite;
		::model::Tetrimino mCurrentTetrimino;
		std::string mNextTetrimino, mNextStage;
		::model::Stage mStage;
	};
}