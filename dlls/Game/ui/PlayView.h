////
// PlayView is UI component for online.
////

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
		PlayView( const bool isPlayable );
		PlayView( const PlayView& another ) = delete;
		PlayView( PlayView&& ) = delete;
		void operator=( const PlayView& ) = delete;
		virtual ~PlayView( ) = default;
		
		static void LoadResources( );
		// Synchronizes with the snapshot on the server.
		void update( std::vector<sf::Event>& eventQueue, ::scene::online::Online& net );
		void draw( sf::RenderWindow& window );
		bool loadCountdownSprite( std::string& filePath )
		{
			if ( false == mTexture_countdown->loadFromFile(filePath) )
			{
				return false;
			}
			else
			{
				mSprite_countdown.setTexture( *mTexture_countdown );
				return true;
			}
		}
		void setCountdownSpriteDimension( const sf::Vector2f origin,
										 const float cellSize,
										 const sf::Vector2i clipSize );
		void setCurrentTetriminoDimension( const sf::Vector2f origin,
										  const float cellSize )
		{
			mCurrentTetrimino.setOrigin( origin );
			mCurrentTetrimino.setSize( cellSize );
		}
		void setStageDimension( const sf::Vector2f position,
							   const float cellSize	)
		{
			mStage.setPosition( position );
			mStage.setSize( cellSize );
		}
		void setStageColor( const sf::Color background, const sf::Color cellOutline )
		{
			mStage.setColor( background, cellOutline );
		}
		void setStageOutline( const float thickness, const sf::Color color )
		{
			mStage.setOutline( thickness, color );
		}
		void getReady( );
		void setCurrentTetrimino( const ::model::tetrimino::Type newCurrentType )
		{
			mCurrentTetrimino = ::model::Tetrimino::Spawn(newCurrentType);
			mHasCurrentTetrimino = true;
			mState = State::PLAYING;
		}
		void moveCurrentTetrimino( const ::model::tetrimino::Rotation rotationID,
								  sf::Vector2<int8_t> destination )
		{
			mCurrentTetrimino.move( rotationID, destination );
		}
		void land( )
		{
			mHasTetriminoLandedOnServer = true;
			resetAlarm( AlarmIndex::GAP_LANDING_ON_SERVER );
		}
		void pushNextTetrimino( const ::model::tetrimino::Type nextType )
		{
			const ::model::Tetrimino& tet =
				mNextTetriminos.emplace_back( ::model::Tetrimino::Spawn(nextType) );
			if ( 1 == mNextTetriminos.size() )
			{
				mNextTetriminoPanel.setTetrimino( tet );
			}
		}
		// Sets stage promptly or pushes it into a buffer if needed.
		void trySetStage( const ::model::stage::Grid& grid )
		{
			if ( true == mIsForThisPlayer &&
				State::PLAYING == mState )
			{
				mBufferForStage = grid;
			}
			else
			{
				mStage.deserialize( grid );
			}
		}
		void playLineClearEffects( const uint8_t numOfLinesCleared );
		void gameOver( )
		{
			mState = State::WAITING_OR_OVER;
			mNextTetriminoPanel.clearTetrimino( );
		}
		void clearStage( )
		{
			mStage.clear( );
		}
		const ::model::Stage& stage( )
		{
			return mStage;
		}
		::vfx::Combo& vfxCombo( )
		{
			return mVfxCombo;
		}
		::ui::NextTetriminoPanel& nextTetriminoPanel( )
		{
			return mNextTetriminoPanel;
		}
	private:
		enum class State
		{
			ON_START,
			PLAYING,
			WAITING_OR_OVER,
		};
		// NOTE: 서버와의 동기화를 위해
		// FPS에 따라 달라지는 frame counter가 아닌 실제 시간을 잽니다.
		enum class AlarmIndex
		{
			COUNT_DOWN,
			INTERVAL_TETRIMINO_DOWN,
			GAP_LANDING_ON_SERVER,
			NONE_MAX,
		};
		enum class SoundIndex
		{
			TETRIMINO_LOCK,
			LINE_CLEAR,
			NONE_MAX,
		};
		// This function doesn't reset the alarm.
		bool alarmAfter( const uint16_t milliseconds, const AlarmIndex index )
		{
			bool elapsed = false;
			if ( std::chrono::milliseconds(milliseconds) < Clock::now()-mAlarms[(int)index] )
			{
				elapsed = true;
			}
			return elapsed;
		}
		void resetAlarm( const AlarmIndex index )
		{
			mAlarms[(int)index] = Clock::now();
		}
		static std::string SoundPaths[(int)SoundIndex::NONE_MAX];
		bool mHasTetriminoLandedOnClient, mHasTetriminoLandedOnServer,
			mIsForThisPlayer, mHasCurrentTetrimino;
		uint8_t mCountDownSec, mNumOfLinesCleared;
		uint16_t mFrameCountInputDelay, mFrameCountVfxDuration, mFPS_, mTempoMs;
		State mState;
		sf::Vector2i countdownSpriteSize;
		// NOTE: Stack-based sf::Texture instance made a malfunction here.
		std::unique_ptr<sf::Texture> mTexture_countdown;
		Clock::time_point mAlarms[(int)AlarmIndex::NONE_MAX];
		sf::Sprite mSprite_countdown;
		::model::Tetrimino mCurrentTetrimino;
		std::list<::model::Tetrimino> mNextTetriminos;
		::vfx::Combo mVfxCombo;
		::model::stage::Grid mBufferForStage;
		::model::Stage mStage;
		::ui::NextTetriminoPanel mNextTetriminoPanel;
	};
}