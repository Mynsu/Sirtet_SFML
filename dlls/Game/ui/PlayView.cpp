#include "../pch.h"
#include "PlayView.h"
#include "../scene/online/Online.h"
#include "../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"

const uint8_t FALLING_DOWN_SPEED = 3;
const uint32_t INPUT_DELAY_MS = 40;
const uint32_t ASYNC_TOLERANCE_MS = 2500;
 
ui::PlayView::PlayView( )
	: mWindow_( nullptr ), mNet( nullptr ), mVfxCombo( *mWindow_), mNextTetriminoPanel( *mWindow_ )
{
}

ui::PlayView::PlayView( sf::RenderWindow& window, ::scene::online::Online& net, const bool isPlayable )
	: mHasTetriminoCollidedOnClient( false ), mHasTetriminoCollidedOnServer( false ),
	mIsForThisPlayer( isPlayable ), mHasCurrentTetrimino( false ),
	mCountDownSec( 3 ), mNumOfLinesCleared( 0 ),
	mFrameCount_input( 0 ), mFrameCount_clearingVFX( 0 ),
	mState_( PlayView::State::WAITING_OR_OVER ),
	mTempoMs( 1000 ),
	mWindow_( &window ), mNet( &net ),
	mAlarms{ Clock::now() },
	mTexture_countdown( std::make_unique<sf::Texture>() ), mVfxCombo( window ),
	mStage( window ), mNextTetriminoPanel( window )
{
}

bool ui::PlayView::loadCountdownSprite( std::string& filePathNName )
{
	if ( false == mTexture_countdown->loadFromFile(filePathNName) )
	{
		return false;
	}
	else
	{
		mSprite.setTexture( *mTexture_countdown );
		return true;
	}
}

void ui::PlayView::setCountdownSpriteDimension( const sf::Vector2f origin,
											   const float cellSize,
											   const sf::Vector2i clipSize )
{
	mSprite.setOrigin( sf::Vector2f(clipSize)*.5f );
	const sf::Vector2f panelSize( ::model::stage::GRID_WIDTH*cellSize, ::model::stage::GRID_HEIGHT*cellSize );
	mSprite.setPosition( origin + panelSize*.5f );
	if ( (int)panelSize.x < clipSize.x )
	{
		mSprite.setScale( 0.5f, 0.5f );
	}
	countdownSpriteSize_ = clipSize;
}

void ui::PlayView::start()
{
	mHasCurrentTetrimino = false;
	while ( false == mNextTetriminos.empty() )
	{
		mNextTetriminos.pop( );
	}
	mTempoMs = 1000;
	resetAlarm( AlarmIndex::COUNT_DOWN );
	mCountDownSec = 3;
	mState_ = PlayView::State::ON_START;
}

void ui::PlayView::update( std::list<sf::Event>& eventQueue )
{
	if ( PlayView::State::ON_START == mState_ &&
		0 == mCountDownSec &&
		true == mHasCurrentTetrimino )
	{
		mState_ = PlayView::State::PLAYING;
	}

	if ( false == mIsForThisPlayer )
	{
		return;
	}

	// Exception
	if ( true == mHasTetriminoCollidedOnServer &&
		true == alarmAfter(ASYNC_TOLERANCE_MS, AlarmIndex::COLLIDED_ON_SERVER) )
	{
		gService()->console().printFailure( FailureLevel::FATAL, "Over asynchronization." );
		mNet->disconnect( );
		return;
	}
	// TODO: InRoom.cpp에 있는 걸 여기로 옮기기
	if ( true == mNet->hasReceived() )
	{
		if ( std::optional<std::string> nextTet( mNet->getByTag(TAG_MY_NEXT_TETRIMINO,
															   ::scene::online::Online::Option::DEFAULT,
																   sizeof(uint8_t)) );
			std::nullopt != nextTet	)
		{
			const ::model::tetrimino::Type type =
				(::model::tetrimino::Type)*nextTet.value().data();
			mNextTetriminos.emplace( ::model::Tetrimino::Spawn(type) );
			if ( true == mHasTetriminoCollidedOnClient )
			{
				mCurrentTetrimino = mNextTetriminos.front();
				mNextTetriminos.pop( );
				mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
			}
			else if ( PlayView::State::ON_START == mState_ )
			{
				mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
			}
		}

		if ( std::optional<std::string> tempoMs( mNet->getByTag(TAG_MY_TEMPO_MS,
																 ::scene::online::Online::Option::DEFAULT,
															   sizeof(uint32_t)) );
			std::nullopt != tempoMs )
		{
			const uint32_t tempo = *(uint32_t*)tempoMs.value().data();
			mTempoMs = ::ntohl(tempo);
		}

		if ( std::optional<std::string> stage( mNet->getByTag(TAG_MY_STAGE,
															 ::scene::online::Online::Option::DEFAULT,
															 sizeof(::model::stage::Grid)) );
			std::nullopt != stage )
		{
			if ( true == mHasTetriminoCollidedOnClient ||
				PlayView::State::WAITING_OR_OVER == mState_ )
			{
 				mStage.deserialize( (::model::stage::Grid*)stage.value().data() );
				mHasTetriminoCollidedOnClient = false;
			}
			else
			{
				mNextStageSerialized = std::move(stage.value());
				mHasTetriminoCollidedOnServer = true;
				resetAlarm( AlarmIndex::COLLIDED_ON_SERVER );
			}
		}
	}

	if ( true == mHasTetriminoCollidedOnClient ||
		PlayView::State::PLAYING != mState_ )
	{
		return;
	}
	
	auto& vault = gService()->vault();
	if ( const auto it = vault.find(HK_HAS_GAINED_FOCUS);
		vault.end() != it )
	{
		if ( 1 == it->second )
		{
			if ( const auto it2 = vault.find(HK_FORE_FPS);
				vault.end() != it2 )
			{
				mFPS_ = it2->second;
			}
#ifdef _DEBUG
			else
			{
				__debugbreak( );
			}
#endif
		}
		else
		{
			if ( const auto it2 = vault.find(HK_BACK_FPS);
				vault.end() != it2 )
			{
				mFPS_ = it2->second;
			}
#ifdef _DEBUG
			else
			{
				__debugbreak( );
			}
#endif
		}
	}
#ifdef _DEBUG
	else
	{
		__debugbreak( );
	}
#endif
	Packet packet;
	if ( true == mCurrentTetrimino.isFallingDown() )
	{
		for ( uint8_t i = 0; FALLING_DOWN_SPEED != i; ++i )
		{
			//TODO: 시간이 많이 지난 만큼 더 이동할까?
			mHasTetriminoCollidedOnClient = mCurrentTetrimino.moveDown(mStage.cgrid());
			if ( true == mHasTetriminoCollidedOnClient )
			{
				mCurrentTetrimino.fallDown( false );
				goto last;
			}
		}
		return;
	}
	else
	{
		const uint32_t inputDelayFPS = mFPS_ * INPUT_DELAY_MS / 1000;
		for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
		{
			if ( sf::Event::KeyPressed == it->type )
			{
				switch ( it->key.code )
				{
					case sf::Keyboard::Space:
						if ( inputDelayFPS < mFrameCount_input )
						{
							mFrameCount_input = 0;
							if ( false == mHasTetriminoCollidedOnServer )
							{
								packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::FALL_DOWN );
							}
							mCurrentTetrimino.fallDown( );
							resetAlarm( AlarmIndex::TETRIMINO_DOWN );
						}
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Down:
						if ( inputDelayFPS < mFrameCount_input )
						{
							mFrameCount_input = 0;
							if ( false == mHasTetriminoCollidedOnServer )
							{
								packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::DOWN );
							}
							mHasTetriminoCollidedOnClient = mCurrentTetrimino.moveDown( mStage.cgrid() );
							resetAlarm( AlarmIndex::TETRIMINO_DOWN );
						}
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Left:
						if ( inputDelayFPS < mFrameCount_input )
						{
							mFrameCount_input = 0;
							if ( false == mHasTetriminoCollidedOnServer )
							{
								packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::LEFT );
								mCurrentTetrimino.tryMoveLeft( mStage.cgrid() );
							}
						}
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Right:
						if ( inputDelayFPS < mFrameCount_input )
						{
							mFrameCount_input = 0;
							if ( false == mHasTetriminoCollidedOnServer )
							{
								packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::RIGHT );
								mCurrentTetrimino.tryMoveRight( mStage.cgrid() );
							}
						}
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::LShift:
						[[ fallthrough ]];
					case sf::Keyboard::Up:
						if ( inputDelayFPS < mFrameCount_input )
						{
							mFrameCount_input = 0;
							if ( false == mHasTetriminoCollidedOnServer )
							{
								packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::ROTATE );
								mCurrentTetrimino.tryRotate( mStage.cgrid() );
							}
						}
						it = eventQueue.erase( it );
						break;
					default:
						++it;
						break;
				}
			}
			else
			{
				++it;
			}
		}

		const Clock::time_point now = Clock::now();
		if ( false == mHasTetriminoCollidedOnClient &&
			alarmAfter(mTempoMs, AlarmIndex::TETRIMINO_DOWN) )
		{
			//TODO: 시간이 많이 지난 만큼 더 이동할까?
			mHasTetriminoCollidedOnClient = mCurrentTetrimino.moveDown( mStage.cgrid() );
			resetAlarm( AlarmIndex::TETRIMINO_DOWN );
		}
	}

	last:
	if ( true == mHasTetriminoCollidedOnClient )
	{
		if ( true == mHasTetriminoCollidedOnServer )
		{
			mHasTetriminoCollidedOnServer = false;
			mHasTetriminoCollidedOnClient = false;
			mCurrentTetrimino = mNextTetriminos.front();
			mNextTetriminos.pop( );
			mStage.deserialize( (::model::stage::Grid*)mNextStageSerialized.data() );
			mNextStageSerialized.clear( );
			if ( false == mNextTetriminos.empty() )
			{
				mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
			}
			// Probably the game is over,
			else
			{
				mNextTetriminoPanel.clearTetrimino( );
			}
		}
#ifdef _DEBUG
		else
		{
			gService()->console().print( "Client has it done while Server hasn't it done yet.",
										sf::Color::Green );
		}
#endif
		const uint8_t ignored = 1;
		packet.pack( TAG_MY_TETRIMINO_COLLIDED_ON_CLIENT, ignored );
	}

	if ( true == packet.hasData() )
	{
		mNet->send( packet );
	}
}

void ui::PlayView::setNewCurrentTetrimino( const::model::tetrimino::Type newCurrentType )
{
	mCurrentTetrimino = ::model::Tetrimino::Spawn(newCurrentType);
	mHasCurrentTetrimino = true;
}

void ui::PlayView::setNumOfLinesCleared( const uint8_t numOfLinesCleared )
{
	mNumOfLinesCleared = numOfLinesCleared;
	mFrameCount_clearingVFX = mFPS_;
}

void ui::PlayView::gameOver()
{
	mState_ = PlayView::State::WAITING_OR_OVER;
	mNextTetriminoPanel.clearTetrimino( );
}

void ui::PlayView::draw()
{
	mStage.draw( );
	if ( true == mIsForThisPlayer )
	{
		mNextTetriminoPanel.draw( );
	}
	if ( PlayView::State::PLAYING == mState_ )
	{
		mCurrentTetrimino.draw( *mWindow_ );
		if ( true == mIsForThisPlayer )
		{
			if ( 0 < mFrameCount_clearingVFX )
			{
				mVfxCombo.draw( mNumOfLinesCleared );
				--mFrameCount_clearingVFX;
			}
			++mFrameCount_input;
		}
	}
	else if ( PlayView::State::ON_START == mState_ )
	{
		if ( 0 != mCountDownSec )
		{
			if ( true == alarmAfter(1000, AlarmIndex::COUNT_DOWN) )
			{
				--mCountDownSec;
				resetAlarm( AlarmIndex::COUNT_DOWN );
			}
			mSprite.setTextureRect( sf::IntRect(0, countdownSpriteSize_.y*(mCountDownSec-1),
												countdownSpriteSize_.x, countdownSpriteSize_.y) );
			mWindow_->draw( mSprite );
		}
	}
}

::model::Tetrimino& ui::PlayView::currentTetrimino()
{
	return mCurrentTetrimino;
}

::model::Stage& ui::PlayView::stage()
{
	return mStage;
}

::vfx::Combo& ui::PlayView::vfxCombo()
{
	return mVfxCombo;
}

::ui::NextTetriminoPanel& ui::PlayView::nextTetriminoPanel()
{
	return mNextTetriminoPanel;
}