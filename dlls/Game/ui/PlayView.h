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
		PlayView( sf::RenderWindow& window, ::scene::online::Online& net, const bool isPlayable = true );
		~PlayView( ) = default;
		
		bool loadCountdownSprite( std::string& filePathNName );
		void setCountdownSpriteDimension( const sf::Vector2f origin,
										 const float cellSize,
										 const sf::Vector2i clipSize );
		void start( );
		void update( std::list<sf::Event>& eventQueue );
		void setNewCurrentTetrimino( const ::model::tetrimino::Type newCurrentType );
		void setNumOfLinesCleared( const uint8_t numOfLinesCleared );
		// When called twice, stage will be cleared.
		void gameOver( );
		void draw( );
		::model::Tetrimino& currentTetrimino( );
		::model::Stage& stage( );
		::vfx::Combo& vfxCombo( );
		::ui::NextTetriminoPanel& nextTetriminoPanel( );
	private:
		enum class AlarmIndex
		{
			COUNT_DOWN,
			TETRIMINO_DOWN,
			COLLIDED_ON_SERVER,
			NONE_MAX,
		};
		// This function doesn't reset the alarm.
		inline bool alarmAfter( const uint32_t milliseconds, const AlarmIndex index )
		{
			bool elapsed = false;
			if ( std::chrono::milliseconds(milliseconds) < Clock::now()-mAlarms[(int)index] )
			{
				elapsed = true;
			}
			return elapsed;
		}
		inline void resetAlarm( const AlarmIndex index )
		{
			mAlarms[(int)index] = Clock::now();
		}
		bool mHasTetriminoCollidedOnClient, mHasTetriminoCollidedOnServer, mIsPlayable;
		uint8_t mCountDownSec, mNumOfLinesCleared;
		uint32_t mFrameCount_input, mFrameCount_clearingVFX, mFrameCount_reset, mFPS_;
		uint32_t mTempoMs;
		enum class State
		{
			ON_START,
			PLAYING,
			WAITING_OR_OVER,
		};
		State mState_;
		sf::Vector2i countdownSpriteSize_;
		sf::RenderWindow* mWindow_;
		::scene::online::Online* mNet;
		// NOTE: Stack-based sf::Texture instance made a malfunction here.
		std::unique_ptr<sf::Texture> mTexture_countdown;
		Clock::time_point mAlarms[(int)AlarmIndex::NONE_MAX];
		sf::Sprite mSprite;
		::model::Tetrimino mCurrentTetrimino;
		std::queue<::model::Tetrimino> mNextTetriminos;
		std::string mNextStageSerialized;
		::model::Stage mStage;
		::vfx::Combo mVfxCombo;
		::ui::NextTetriminoPanel mNextTetriminoPanel;
	};
}