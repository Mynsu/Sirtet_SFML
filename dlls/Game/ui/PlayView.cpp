#include "../pch.h"
#include "PlayView.h"
#include "../scene/online/Online.h"
#include "../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"

const uint8_t FALLING_DIFF = 3;
const uint32_t INPUT_DELAY_MS = 40;
const uint32_t ASYNC_TOLERANCE_MS = 1000;
 
ui::PlayView::PlayView( )
	: mWindow_( nullptr ), mNet( nullptr ), mVfxCombo( *mWindow_), mNextTetriminoPanel( *mWindow_ )
{
}

ui::PlayView::PlayView( sf::RenderWindow& window, ::scene::online::Online& net )
	: mIsOnStart( true ), mHasTetriminoCollidedOnClient( false ),
	mIsGameOverOnServer( false ),
	mNumOfLinesCleared( 0 ),
	mFrameCount_collisionOnServer( 0 ), mFrameCount_input( 0 ),
	mFrameCount_countdown( 0 ), mFrameCount_clearingVFX( 0 ),
	mTempoMs( std::chrono::milliseconds(1000) ),
	mWindow_( &window ), mNet( &net ),
	mTexture_countdown( std::make_unique<sf::Texture>() ), mVfxCombo( window ),
	mStage( window ), mNextTetriminoPanel( window )
{
	auto& vault = gService()->vault();
	if ( const auto it = vault.find(HK_FORE_FPS);
		vault.end() != it )
	{
		mFPS_ = it->second;
	}
#ifdef _DEBUG
	else
	{
		__debugbreak( );
	}
#endif
}

bool ui::PlayView::loadTexture( std::string& filePathNName )
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
											   const sf::Vector2i size)
{
	mSprite.setOrigin( sf::Vector2f(size)*.5f );
	const sf::Vector2f panelSize( ::model::stage::GRID_WIDTH, ::model::stage::GRID_HEIGHT );
	mSprite.setPosition( origin + panelSize*cellSize*.5f );
	countdownSpriteSize_ = size;
}

void ui::PlayView::start()
{
	mFrameCount_countdown = 180;
}

void ui::PlayView::update( std::list<sf::Event>& eventQueue )
{
	Packet packet;
	if ( true == mIsGameOverOnServer )
	{
		return;
	}
	// Exception
	else if ( mFPS_*ASYNC_TOLERANCE_MS/1000 < mFrameCount_collisionOnServer )
	{
		gService()->console().printFailure( FailureLevel::FATAL, "Over asynchronization." );
		mNet->disconnect( );
		return;
	}
	else if ( true == mNet->hasReceived() )
	{
		if ( std::optional<std::string> newCurTet(mNet->getByTag(TAG_MY_NEW_CURRENT_TETRIMINO,
																::scene::online::Online::Option::SPECIFIED_SIZE,
																  sizeof(uint8_t)));
			std::nullopt != newCurTet )
		{
			const ::model::tetrimino::Type type = (::model::tetrimino::Type)*newCurTet.value().data();
			setNewCurrentTetrimino( type );
		}

		if ( std::optional<std::string> nextTet(mNet->getByTag(TAG_MY_NEXT_TETRIMINO,
															   ::scene::online::Online::Option::SPECIFIED_SIZE,
																   sizeof(uint8_t)));
			std::nullopt != nextTet	)
		{
			const ::model::tetrimino::Type type =
				(::model::tetrimino::Type)*nextTet.value().data();
			mNextTetriminoS.emplace( ::model::Tetrimino::Spawn(type) );
			if ( true == mHasTetriminoCollidedOnClient )
			{
				mCurrentTetrimino = mNextTetriminoS.front();
				mNextTetriminoS.pop( );
				mNextTetriminoPanel.setTetrimino( mNextTetriminoS.front() );
			}
			else if ( true == mIsOnStart )
			{
				mNextTetriminoPanel.setTetrimino( mNextTetriminoS.front() );
				mIsOnStart = false;
			}
		}

		if ( std::optional<std::string> tempoMs(mNet->getByTag(TAG_MY_TEMPO_MS,
																 ::scene::online::Online::Option::SPECIFIED_SIZE,
															   sizeof(uint32_t)));
			std::nullopt != tempoMs )
		{
			
			const uint32_t tempo = *(uint32_t*)tempoMs.value().data();
			mTempoMs = std::chrono::milliseconds( ::ntohl(tempo) );
		}

		if ( std::optional<std::string> stage(mNet->getByTag(TAG_MY_STAGE,
															 ::scene::online::Online::Option::SPECIFIED_SIZE,
															 sizeof(::model::stage::Grid)));
			std::nullopt != stage )
		{
			if ( true == mHasTetriminoCollidedOnClient ||
				true == mIsGameOverOnServer )
			{
 				mStage.deserialize( (::model::stage::Grid*)stage.value().data() );
				mHasTetriminoCollidedOnClient = false;
			}
			else
			{
				mNextStageSerialized = std::move( stage.value() );
				mFrameCount_collisionOnServer = 1;
			}
		}

		if ( std::optional<std::string> numOfLinesClearedOnServer(mNet->getByTag(TAG_MY_NUM_OF_LINES_CLEARED,
																		 ::scene::online::Online::Option::SPECIFIED_SIZE,
																				 sizeof(uint8_t)) );
			std::nullopt != numOfLinesClearedOnServer )
		{
			setNumOfLinesCleared((uint8_t)*numOfLinesClearedOnServer.value().data());
		}

		if ( std::optional<std::string> isGameOverOnServer(mNet->getByTag(TAG_MY_GAME_OVER,
																		  ::scene::online::Online::Option::RETURN_TAG_ATTACHED,
																		  sizeof(uint8_t)) );
			std::nullopt != isGameOverOnServer )
		{
			gameOverOnServer();
			return;
		}
	}
	
	if ( true == mHasTetriminoCollidedOnClient || true == mIsOnStart )
	{
		return;
	}
	else if ( true == mCurrentTetrimino.isFallingDown() )
	{
		for ( uint8_t i = 0; FALLING_DIFF != i; ++i )
		{
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
							if ( 0 == mFrameCount_collisionOnServer )
							{
								packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::FALL_DOWN );
							}
							mCurrentTetrimino.fallDown( );
							mLastTimeFallingdown = Clock::now();
						}
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Down:
						if ( inputDelayFPS < mFrameCount_input )
						{
							mFrameCount_input = 0;
							if ( 0 == mFrameCount_collisionOnServer )
							{
								packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::DOWN );
							}
							mHasTetriminoCollidedOnClient = mCurrentTetrimino.moveDown( mStage.cgrid() );
							mLastTimeFallingdown = Clock::now();
						}
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Left:
						if ( inputDelayFPS < mFrameCount_input )
						{
							mFrameCount_input = 0;
							if ( 0 == mFrameCount_collisionOnServer )
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
							if ( 0 == mFrameCount_collisionOnServer )
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
							if ( 0 == mFrameCount_collisionOnServer )
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
			mTempoMs <= now-mLastTimeFallingdown )
		{
			mHasTetriminoCollidedOnClient = mCurrentTetrimino.moveDown( mStage.cgrid() );
			mLastTimeFallingdown = now;
		}
	}

	last:
	if ( true == mHasTetriminoCollidedOnClient )
	{
		if ( 0 < mFrameCount_collisionOnServer )
		{
			mFrameCount_collisionOnServer = 0;
			mHasTetriminoCollidedOnClient = false;
			mCurrentTetrimino = mNextTetriminoS.front();
			mNextTetriminoS.pop( );
			mStage.deserialize( (::model::stage::Grid*)mNextStageSerialized.data() );
			mNextStageSerialized.clear( );
			mNextTetriminoPanel.setTetrimino( mNextTetriminoS.front() );
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
}

void ui::PlayView::setNumOfLinesCleared( const uint8_t numOfLinesCleared )
{
	mNumOfLinesCleared = numOfLinesCleared;
	mFrameCount_clearingVFX = mFPS_;
}

void ui::PlayView::gameOverOnServer()
{
	mIsGameOverOnServer = true;
	mIsOnStart = true;
}

void ui::PlayView::draw( )
{
	mStage.draw( );
	if ( false == mIsOnStart )
	{
		mCurrentTetrimino.draw( *mWindow_ );
		mNextTetriminoPanel.draw( );
	}
	else if ( 0 < mFrameCount_countdown )
	{
		--mFrameCount_countdown;
		const uint8_t y = mFrameCount_countdown/mFPS_;
		mSprite.setTextureRect( sf::IntRect( 0, countdownSpriteSize_.y*y,
											countdownSpriteSize_.x, countdownSpriteSize_.y ) );
		mWindow_->draw( mSprite );
	}
	
	if ( 0 < mFrameCount_clearingVFX )
	{
		mVfxCombo.draw( mNumOfLinesCleared );
		--mFrameCount_clearingVFX;
	}
	
	if ( true == mIsGameOverOnServer )
	{

	}

	++mFrameCount_input;
	if ( 0 != mFrameCount_collisionOnServer )
	{
		++mFrameCount_collisionOnServer;
	}
}

::model::Tetrimino& ui::PlayView::tetrimino()
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