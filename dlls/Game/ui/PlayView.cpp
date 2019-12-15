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
	: mHasTetrimino( false ), mHasTetriminoCollidedAlready( false ),
	mIsGameOverOnServer( false ),
	mNumOfLinesCleared( 0 ),
	mFrameCount_collisionOnServer( 0 ), mFrameCount_input( 0 ), mFrameCount_clearingVFX( 0 ),
	mCellSize( 30.f ), mTempoMs( std::chrono::milliseconds(1000) ),
	mWindow_( &window ), mNet( &net ),
	mTexture( std::make_unique<sf::Texture>() ),
	mStage( window )
{
	if ( false == mTexture->loadFromFile("Images/Ready.png") )
	{
		gService()->console().printFailure( FailureLevel::WARNING, "Can't find the countdown image." );
	}
	mSprite.setTexture( *mTexture );
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
	if ( true == mIsGameOverOnServer )
	{
		return false;
	}

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
	if ( fps*ASYNC_TOLERANCE_MS/1000 < mFrameCount_collisionOnServer )
	{
		mNet->disconnect( );
		gService()->console().printFailure( FailureLevel::FATAL, "Over asynchronization." );
		return false;
	}

	bool hasToRespond = false;
	if ( true == mNet->hasReceived() )
	{
		if ( std::optional<std::string> isGameOverOnServer(mNet->getByTag(TAG_MY_GAME_OVER,
																	   ::scene::online::Online::Option::RETURN_TAG_ATTACHED,
																		  sizeof(uint8_t)) );
			std::nullopt != isGameOverOnServer )
		{
			mIsGameOverOnServer = true;
			mHasTetrimino = false;
		}

		if ( std::optional<std::string> curTetType(mNet->getByTag(TAG_MY_CURRENT_TETRIMINO,
																::scene::online::Online::Option::SPECIFIED_SIZE,
																  sizeof(::model::tetrimino::Type)));
			std::nullopt != curTetType )
		{
			const ::model::tetrimino::Type type =
				(::model::tetrimino::Type)::ntohl(*(u_long*)curTetType.value().data());
			mCurrentTetrimino = ::model::Tetrimino::Spawn( type );
			mHasTetrimino = true;
		}

		if ( std::optional<std::string> nextTetType(mNet->getByTag(TAG_MY_NEXT_TETRIMINO,
															   ::scene::online::Online::Option::SPECIFIED_SIZE,
																   sizeof(::model::tetrimino::Type)));
			std::nullopt != nextTetType	)
		{
			const ::model::tetrimino::Type type =
				(::model::tetrimino::Type)::ntohl(*(u_long*)nextTetType.value().data());
			const ::model::Tetrimino nextTet( ::model::Tetrimino::Spawn(type) );
			mNextTetriminoS.emplace( nextTet );
			if ( true == mHasTetriminoCollidedAlready )
			{
				mCurrentTetrimino = mNextTetriminoS.front();
				mNextTetriminoS.pop( );
			}
		}

		if ( std::optional<std::string> tempoMs(mNet->getByTag(TAG_MY_TEMPO_MS,
																 ::scene::online::Online::Option::SPECIFIED_SIZE,
															   sizeof(uint32_t)));
			std::nullopt != tempoMs )
		{
			uint32_t* const p = (uint32_t*)tempoMs.value().data();
			mTempoMs = std::chrono::milliseconds( (uint32_t)::ntohl((u_long)*p) );
		}

		if ( std::optional<std::string> stageOnServer(mNet->getByTag(TAG_MY_STAGE,
																  ::scene::online::Online::Option::INDETERMINATE_SIZE));
			std::nullopt != stageOnServer )
		{
			if ( true == mHasTetriminoCollidedAlready || true == mIsGameOverOnServer )
			{
 				mStage.updateOnNet( stageOnServer.value() );
				mHasTetriminoCollidedAlready = false;
			}
			else
			{
				mNextStage = std::move( stageOnServer.value() );
				mFrameCount_collisionOnServer = 1;
			}
		}

		if ( std::optional<std::string> numOfLinesClearedOnServer(mNet->getByTag(TAG_MY_LINES_CLEARED,
																		 ::scene::online::Online::Option::SPECIFIED_SIZE,
																				 sizeof(uint8_t)) );
			std::nullopt != numOfLinesClearedOnServer )
		{
			mNumOfLinesCleared = (uint8_t)*numOfLinesClearedOnServer.value().data();
		}

		hasToRespond = true;
	}
	else if ( true == mHasTetriminoCollidedAlready )
	{
		return false;
	}

	Packet packet;
	if ( false == mHasTetrimino )
	{
		return hasToRespond;
	}
	else if ( true == mCurrentTetrimino.isFallingDown() )
	{
		for ( uint8_t i = 0; FALLING_DIFF != i; ++i )
		{
			mHasTetriminoCollidedAlready = mCurrentTetrimino.moveDown(mStage.cgrid());
			if ( true == mHasTetriminoCollidedAlready )
			{
				mCurrentTetrimino.fallDown( false );
				goto last;
			}
		}
		return hasToRespond;
	}
	else
	{
		const uint32_t inputDelayFPS = fps * INPUT_DELAY_MS / 1000;
		for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it && inputDelayFPS < mFrameCount_input; )
		{
			if ( sf::Event::KeyPressed == it->type )
			{
				switch ( it->key.code )
				{
					case sf::Keyboard::Space:
						if ( 0 == mFrameCount_collisionOnServer )
						{
							packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::FALL_DOWN );
						}
						mCurrentTetrimino.fallDown( );
						mPast_falldown = Clock::now();
						mFrameCount_input = 0;
						break;
					case sf::Keyboard::Down:
						if ( 0 == mFrameCount_collisionOnServer )
						{
							packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::DOWN );
						}
						mHasTetriminoCollidedAlready = mCurrentTetrimino.moveDown( mStage.cgrid() );
						mPast_falldown = Clock::now();
						mFrameCount_input = 0;
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Left:
						if ( 0 == mFrameCount_collisionOnServer )
						{
							packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::LEFT );
							mCurrentTetrimino.tryMoveLeft( mStage.cgrid() );
						}
						mFrameCount_input = 0;
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Right:
						if ( 0 == mFrameCount_collisionOnServer )
						{
							packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::RIGHT );
							mCurrentTetrimino.tryMoveRight( mStage.cgrid() );
						}
						mFrameCount_input = 0;
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::LShift:
						[[ fallthrough ]];
					case sf::Keyboard::Up:
						if ( 0 == mFrameCount_collisionOnServer )
						{
							packet.pack( TAG_MY_TETRIMINO_MOVE, (uint8_t)::model::tetrimino::Move::ROTATE );
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

		Clock::time_point now =	Clock::now();
		if ( mTempoMs <= now-mPast_falldown )
		{
			mHasTetriminoCollidedAlready = mCurrentTetrimino.moveDown( mStage.cgrid() );
			mPast_falldown = now;
		}
	}

	last:
	if ( true == mHasTetriminoCollidedAlready )
	{
		if ( 0 < mFrameCount_collisionOnServer )
		{
			mFrameCount_collisionOnServer = 0;
			mHasTetriminoCollidedAlready = false;
			mCurrentTetrimino = mNextTetriminoS.front();
			mNextTetriminoS.pop( );
			mStage.updateOnNet( mNextStage );
			mNextStage.clear( );
			mFrameCount_clearingVFX = 1;
#ifdef _DEBUG
			gService()->console().print( "Server has it done while Client hasn't it done yet.",
										sf::Color::Red );
#endif
		}
		else
		{
#ifdef _DEBUG
			gService()->console().print( "Client has it done while Server hasn't it done yet.",
										sf::Color::Green );
#endif
		}

		packet.pack( TAG_MY_TETRIMINO_COLLIDED_IN_CLIENT, (uint8_t)true );
	}

	if ( true == packet.hasData() )
	{
		mNet->send( packet );
		hasToRespond = false;
	}

	return hasToRespond;
}

void ui::PlayView::draw( const int time )
{
	mStage.draw( );
	if ( time < -1 )
	{
		mSprite.setTextureRect( sf::IntRect( 0, 256*(-time-1), 256, 256 ) );
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
	if ( true == mIsGameOverOnServer )
	{

	}

	++mFrameCount_input;
	if ( 0 != mFrameCount_collisionOnServer )
	{
		++mFrameCount_collisionOnServer;
	}
	const uint32_t fps = (uint32_t)gService()->vault()[HK_FORE_FPS];
	if ( fps <= mFrameCount_clearingVFX )
	{
		mFrameCount_clearingVFX = 0u;
	}
}