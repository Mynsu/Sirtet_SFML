#include "../pch.h"
#include "PlayView.h"
#include "../scene/online/Online.h"
#include "../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"

const uint8_t FALLING_DIFF = 3;
const uint32_t INPUT_DELAY_MS = 40;
const uint32_t ASYNC_TOLERANCE_MS = 1000;
 
ui::PlayView::PlayView( )
	: mCellSize( 0.f ),
	mWindow_( nullptr ), mNet( nullptr )
{
}

ui::PlayView::PlayView( sf::RenderWindow& window, ::scene::online::Online& net )
	: mHasTetrimino( false ), mHasTetriminoCollidedInClient( false ),
	mIsGameOver( false ),
	mNumOfLinesCleared( 0 ),
	mFrameCount_sync( 0 ), mFrameCount_input( 0 ), mFrameCount_clearingVFX( 0 ),
	mCellSize( 30.f ), mTempoMs( std::chrono::milliseconds(1000) ),
	mWindow_( &window ), mNet( &net ), mStage( window )
{
	if ( false == mTexture.loadFromFile("Images/Ready.png") )
	{
		gService()->console().printFailure( FailureLevel::WARNING, "Can't find the countdown image." );
	}
	mSprite.setTexture( mTexture );
	mSprite.setOrigin( 128.f, 128.f );
}

void ui::PlayView::setDimension( const sf::Vector2f position, const float cellSize )
{
	mCellSize = cellSize;
	const sf::Vector2f size( sf::Vector2f(::model::stage::GRID_WIDTH, ::model::stage::GRID_HEIGHT)*cellSize );
	mSprite.setPosition( position + size*.5f );
	mCurrentTetrimino.setOrigin( position );
	mCurrentTetrimino.setSize( cellSize );
	mStage.setPosition( position );
	mStage.setSize( cellSize );
}

bool ui::PlayView::update( std::list<sf::Event>& eventQueue )
{
	uint32_t fps;
	if ( 1 == gService()->vault()[HK_HAS_GAINED_FOCUS] )
	{
		fps = gService()->vault()[HK_FORE_FPS];
	}
	else
	{
		fps = gService()->vault()[HK_BACK_FPS];
	}
	// Exception
	if ( fps*ASYNC_TOLERANCE_MS/1000 < mFrameCount_sync )
	{
		mNet->disconnect( );
		gService()->console().printFailure( FailureLevel::FATAL, "Over asynchronization." );
		return false;
	}

	bool hasToRespond = false;
	if ( true == mNet->hasReceived() )
	{
		if ( std::optional<std::string> curTet(mNet->getByTag(TAG_MY_CURRENT_TETRIMINO,
																::scene::online::Online::Option::SERIALIZED));
			std::nullopt != curTet )
		{
			if ( true == mHasTetriminoCollidedInClient || false == mHasTetrimino )
			{
				mCurrentTetrimino.updateOnNet( curTet.value() );
				mHasTetrimino = true;
			}
			else
			{
				mNextTetrimino = std::move( curTet.value() );
#ifdef _DEBUG
				gService()->console().print( "Server has it done while Client hasn't it done yet.",
											sf::Color::Red );
#endif
			}
		}

		if ( std::optional<std::string> tempoMs(mNet->getByTag(TAG_MY_TEMPO_MS,
																 ::scene::online::Online::Option::SERIALIZED));
			std::nullopt != tempoMs )
		{
			uint32_t* const p = (uint32_t*)tempoMs.value().data();
			mTempoMs = std::chrono::milliseconds( (uint32_t)::ntohl((u_long)*p) );
		}

		if ( std::optional<std::string> stageOnNet(mNet->getByTag(TAG_MY_STAGE,
																  ::scene::online::Online::Option::SERIALIZED));
			std::nullopt != stageOnNet )
		{
			if ( true == mHasTetriminoCollidedInClient )
			{
				mStage.updateOnNet( stageOnNet.value() );
				mHasTetriminoCollidedInClient = false;
				mFrameCount_sync = 0;
			}
			else
			{
				mNextStage = std::move( stageOnNet.value() );
				mFrameCount_sync = 1;
#ifdef _DEBUG
				gService()->console().print( "Server has it done while Client hasn't it done yet.",
											sf::Color::Red );
#endif
			}
		}

		if ( std::optional<std::string> numOfLinesClearedOnNet(mNet->getByTag(TAG_MY_LINES_CLEARED,
																		 ::scene::online::Online::Option::NONE) );
			std::nullopt != numOfLinesClearedOnNet )
		{
			mNumOfLinesCleared = (uint8_t)*numOfLinesClearedOnNet.value().data();
			mFrameCount_clearingVFX = 1;
		}

		if ( std::optional<std::string> isGameOverOnNet(mNet->getByTag(TAG_MY_GAME_OVER,
																		 ::scene::online::Online::Option::RETURN_TAG_ATTACHED) );
			std::nullopt != isGameOverOnNet )
		{
			mHasTetrimino = false;
			mIsGameOver = true;
		}

		hasToRespond = true;
	}
	else if ( true == mHasTetriminoCollidedInClient )
	{
		return hasToRespond;
	}

	if ( false == mHasTetrimino )
	{
		return hasToRespond;
	}
	else if ( true == mCurrentTetrimino.isFallingDown() )
	{
		for ( uint8_t i = 0; FALLING_DIFF != i; ++i )
		{
			mHasTetriminoCollidedInClient = mCurrentTetrimino.moveDown(mStage.cgrid());
			if ( true == mHasTetriminoCollidedInClient )
			{
				mCurrentTetrimino.fallDown( false );
				Packet async;
				async.pack( TAG_MY_SYNC, (uint8_t)false );
				mNet->send( async );
				hasToRespond = false;
				goto last;
			}
		}

		if ( 0 < mFrameCount_sync )
		{
			Packet async;
			async.pack( TAG_MY_SYNC, (uint8_t)true );
			mNet->send( async );
			hasToRespond = false;
			return hasToRespond;
		}
		else
		{
			return hasToRespond;
		}
	}
	else
	{
		const uint32_t inputDelayFPS = fps * INPUT_DELAY_MS / 1000;
		std::string input;
		for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it && inputDelayFPS < mFrameCount_input; )
		{
			if ( sf::Event::KeyPressed == it->type )
			{
				switch ( it->key.code )
				{
					case sf::Keyboard::Space:
						if ( 0 == mFrameCount_sync )
						{
							input += TAGGED_MY_TETRIMINO_MOVE_FALLDOWN;
						}
						mCurrentTetrimino.fallDown( );
						mPast_falldown = Clock::now();
						mFrameCount_input = 0;
						break;
					case sf::Keyboard::Down:
						if ( 0 == mFrameCount_sync )
						{
							input += TAGGED_MY_TETRIMINO_MOVE_DOWN;
						}
						mHasTetriminoCollidedInClient = mCurrentTetrimino.moveDown( mStage.cgrid() );
						mPast_falldown = Clock::now();
						mFrameCount_input = 0;
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Left:
						if ( 0 == mFrameCount_sync )
						{
							input += TAGGED_MY_TETRIMINO_MOVE_LEFT;
							mCurrentTetrimino.tryMoveLeft( mStage.cgrid() );
						}
						mFrameCount_input = 0;
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Right:
						if ( 0 == mFrameCount_sync )
						{
							input += TAGGED_MY_TETRIMINO_MOVE_RIGHT;
							mCurrentTetrimino.tryMoveRight( mStage.cgrid() );
						}
						mFrameCount_input = 0;
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::LShift:
						[[ fallthrough ]];
					case sf::Keyboard::Up:
						if ( 0 == mFrameCount_sync )
						{
							input += TAGGED_MY_TETRIMINO_MOVE_ROTATE;
							mCurrentTetrimino.tryRotate( mStage.cgrid() );
						}
						mFrameCount_input = 0;
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

		if ( const size_t inputSize = input.size(); 0 < inputSize )
		{
			mNet->send( input.data(), (int)inputSize );
			hasToRespond = false;
		}

		Clock::time_point now =	Clock::now();
		if ( mTempoMs <= now-mPast_falldown )
		{
			mHasTetriminoCollidedInClient = mCurrentTetrimino.moveDown( mStage.cgrid() );
			mPast_falldown = now;
		}
	}

	last:
	if ( true == mHasTetriminoCollidedInClient )
	{
		if ( 0 < mFrameCount_sync )
		{
			mCurrentTetrimino.updateOnNet( mNextTetrimino );
			mNextTetrimino.clear( );
			mStage.updateOnNet( mNextStage );
			mNextStage.clear( );
			mHasTetriminoCollidedInClient = false;
			mFrameCount_sync = 0;
		}
		else
		{
#ifdef _DEBUG
			gService()->console().print( "Client has it done while Server hasn't it done yet.",
										sf::Color::Green );
#endif
		}
	}

	return hasToRespond;
}

void ui::PlayView::draw( const int time )
{
	mStage.draw( );
	if ( time < 0 )
	{
		//mSprite.setTextureRect( sf::IntRect( 0, 256*(-time-1), 256, 256 ) );
		mWindow_->draw( mSprite );
	}

	if ( true == mHasTetrimino )
	{
		mCurrentTetrimino.draw( *mWindow_ );
	}
	if ( 0 < mFrameCount_clearingVFX )
	{

		++mFrameCount_clearingVFX;
	}
	if ( true == mIsGameOver )
	{

	}

	++mFrameCount_input;
	if ( 0 != mFrameCount_sync )
	{
		++mFrameCount_sync;
	}
	const uint32_t fps = (uint32_t)gService()->vault()[HK_FORE_FPS];
	if ( fps <= mFrameCount_clearingVFX )
	{
		mFrameCount_clearingVFX = 0u;
	}
}