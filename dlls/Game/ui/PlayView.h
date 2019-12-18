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
		
		bool loadTexture( std::string& filePathNName )
		{
			if ( false == mTexture->loadFromFile(filePathNName) )
			{
				return false;
			}
			else
			{
				mSprite.setTexture( *mTexture );
				return true;
			}
		}
		void setCountdownSpriteDimension( const sf::Vector2f origin, const float cellSize,
										 const sf::Vector2i size )
		{
			mSprite.setOrigin( sf::Vector2f(size)*.5f );
			const sf::Vector2f panelSize( ::model::stage::GRID_WIDTH, ::model::stage::GRID_HEIGHT );
			mSprite.setPosition( origin + panelSize*cellSize*.5f );
			countdownSpriteSize_ = size;
		}
		::model::Stage& stage( )
		{
			return mStage;
		}
		::model::Tetrimino& tetrimino( )
		{
			return mCurrentTetrimino;
		}
		::vfx::Combo& vfxCombo( )
		{
			return mVfxCombo;
		}
		::ui::NextTetriminoPanel& nextTetriminoPanel( )
		{
			return mNextTetriminoPanel;
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
		sf::Vector2i countdownSpriteSize_;
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