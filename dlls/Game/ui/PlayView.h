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
		// NOTE: DO NOT USE.  Not deleted because std::unordered_map requires.
		PlayView( );
		// NOTE: DO NOT USE.  Not deleted because std::unordered_map requires.
		PlayView( PlayView&& ) = default;
		PlayView( sf::RenderWindow& window, ::scene::online::Online& net );
		~PlayView( ) = default;
		
		void setDimension( const sf::Vector2f position, const float cellSize );
		bool update( std::list<sf::Event>& eventQueue );
		void draw( const int time );
	private:
		using Clock = std::chrono::high_resolution_clock;
		bool mHasTetrimino, mHasTetriminoCollidedAlready, mIsGameOverOnServer;
		uint8_t mNumOfLinesCleared;
		// NOTE: Also as mHasTetriminoCollidedIn-Server.
		uint32_t mFrameCount_collisionOnServer;
		uint32_t mFrameCount_input, mFrameCount_clearingVFX;
		float mCellSize;
		Clock::time_point mPast_falldown;
		Clock::duration	mTempoMs;
		sf::RenderWindow* mWindow_;
		::scene::online::Online* mNet;
		// NOTE: Stack-based sf::Texture instance makes a malfunction.
		std::unique_ptr< sf::Texture > mTexture;
		sf::Sprite mSprite;
		::model::Tetrimino mCurrentTetrimino;
		std::queue<::model::Tetrimino> mNextTetriminoS;
		std::string mNextStage;
		::model::Stage mStage;
	};
}