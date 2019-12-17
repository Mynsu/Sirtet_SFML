#pragma once
#include "../model/Tetrimino.h"
#include "../ui/NextTetriminoPanel.h"
#include "../vfx/Combo.h"

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
		
		void loadResource( const sf::Vector2f position, const float cellSize );
		void setNextTetriminoPanelDimension( const sf::Vector2f position, const float cellSize )
		{
			mNextTetriminoPanel.setDimension( position, cellSize );
		}
		void start( )
		{
			mFrameCount_countdown = 180;
		}
		bool update( std::list<sf::Event>& eventQueue );
		void draw( );
	private:
		using Clock = std::chrono::high_resolution_clock;
		bool mHasTetriminos, mHasTetriminoCollidedAlready, mIsGameOverOnServer;
		uint8_t mNumOfLinesCleared;
		// NOTE: Also as mHasTetriminoCollidedIn-Server.
		uint32_t mFrameCount_collisionOnServer;
		uint32_t mFrameCount_countdown, mFrameCount_input, mFrameCount_clearingVFX, mFPS_;
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
		::vfx::Combo mVfxCombo;
		::ui::NextTetriminoPanel mNextTetriminoPanel;
	};
}