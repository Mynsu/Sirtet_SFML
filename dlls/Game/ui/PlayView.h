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
		
		bool loadTexture( std::string& filePathNName );
		void setCountdownSpriteDimension( const sf::Vector2f origin,
										 const float cellSize,
										 const sf::Vector2i size );
		void start( );
		void update( std::list<sf::Event>& eventQueue );
		void setNewCurrentTetrimino( const ::model::tetrimino::Type newCurrentType );
		void setNumOfLinesCleared( const uint8_t numOfLinesCleared );
		void gameOverOnServer( );
		void draw( );
		::model::Tetrimino& tetrimino( );
		::model::Stage& stage( );
		::vfx::Combo& vfxCombo( );
		::ui::NextTetriminoPanel& nextTetriminoPanel( );
	private:
		bool mIsOnStart, mHasTetriminoCollidedOnClient, mIsGameOverOnServer;
		uint8_t mNumOfLinesCleared;
		// NOTE: Also as boolean mHasTetriminoCollidedOnServer.
		uint32_t mFrameCount_collisionOnServer;
		uint32_t mFrameCount_countdown, mFrameCount_input, mFrameCount_clearingVFX, mFPS_;
		sf::Vector2i countdownSpriteSize_;
		Clock::time_point mLastTimeFallingdown;
		Clock::duration	mTempoMs;
		sf::RenderWindow* mWindow_;
		::scene::online::Online* mNet;
		// NOTE: Stack-based sf::Texture instance makes a malfunction.
		std::unique_ptr<sf::Texture> mTexture_countdown;
		sf::Sprite mSprite;
		::model::Tetrimino mCurrentTetrimino;
		std::queue<::model::Tetrimino> mNextTetriminoS;
		std::string mNextStageSerialized;
		::model::Stage mStage;
		::vfx::Combo mVfxCombo;
		::ui::NextTetriminoPanel mNextTetriminoPanel;
	};
}