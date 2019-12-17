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
	: mHasTetriminos( false ), mHasTetriminoCollidedAlready( false ),
	mIsGameOverOnServer( false ),
	mNumOfLinesCleared( 0 ),
	mFrameCount_collisionOnServer( 0 ), mFrameCount_input( 0 ),
	mFrameCount_countdown( 0 ), mFrameCount_clearingVFX( 0 ),
	mTempoMs( std::chrono::milliseconds(1000) ),
	mWindow_( &window ), mNet( &net ),
	mTexture( std::make_unique<sf::Texture>() ), mVfxCombo( window ),
	mStage( window ), mNextTetriminoPanel( window )
{
}

void ui::PlayView::loadResource( const sf::Vector2f position, const float cellSize )
{
	sf::Vector2f countdownSpriteSize( 256.f, 256.f );
	sf::Vector2i vfxComboSize(256, 256);
	
	lua_State* lua = luaL_newstate( );
	const char scriptPathNName[] = "Scripts/PlayView.lua";
	if ( true == luaL_dofile(lua, scriptPathNName) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		const std::string tableName0( "CountdownSprite" );
		lua_getglobal( lua, tableName0.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName0.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "path";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TSTRING == type )
			{
				if ( false == mTexture->loadFromFile(lua_tostring(lua, TOP_IDX)) )
				{
					// File Not Found Exception
					gService( )->console( ).printScriptError( ExceptionType::FILE_NOT_FOUND,
						(tableName0+":"+field0).data(), scriptPathNName );
					const char defaultFilePathNName[] = "Images/Ready.png";
					if ( false == mTexture->loadFromFile(defaultFilePathNName) )
					{
						// Exception: When there's not even the default file,
						gService( )->console( ).printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+defaultFilePathNName );
#ifdef _DEBUG
						__debugbreak( );
#endif
					}
				}
			}
			// Type Check Exception
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				countdownSpriteSize.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "height";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				countdownSpriteSize.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		

		const std::string tableName1( "VfxCombo" );
		lua_getglobal( lua, tableName1.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName1.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "path";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TSTRING == type )
			{
				if ( false == mVfxCombo.loadResources(lua_tostring(lua, TOP_IDX)) )
				{
					// File Not Found Exception
					gService( )->console( ).printScriptError( ExceptionType::FILE_NOT_FOUND,
						(tableName1+":"+field0).data(), scriptPathNName );
					const char defaultFilePathNName[] = "Vfxs/Combo.png";
					if ( false == mVfxCombo.loadResources(defaultFilePathNName) )
					{
						// Exception: When there's not even the default file,
						gService( )->console( ).printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+defaultFilePathNName );
#ifdef _DEBUG
						__debugbreak( );
#endif
					}
				}
			}
			// Type Check Exception
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName1+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				vfxComboSize.x = (int)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName1+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "height";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				vfxComboSize.y = (int)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName1+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	mSprite.setTexture( *mTexture );
	mSprite.setOrigin( countdownSpriteSize*.5f );
	const sf::Vector2f size( sf::Vector2f(::model::stage::GRID_WIDTH, ::model::stage::GRID_HEIGHT)*cellSize );
	mSprite.setPosition( position + size*.5f );
	mCurrentTetrimino.setOrigin( position );
	mCurrentTetrimino.setSize( cellSize );
	mStage.setPosition( position );
	mStage.setSize( cellSize );
	mVfxCombo.setOrigin( position, cellSize, vfxComboSize );
}

bool ui::PlayView::update( std::list<sf::Event>& eventQueue )
{
	if ( true == mIsGameOverOnServer )
	{
		return false;
	}

	if ( 1 == gService()->vault()[HK_HAS_GAINED_FOCUS] )
	{
		mFPS_ = gService()->vault()[HK_FORE_FPS];
	}
	else
	{
		mFPS_ = gService()->vault()[HK_BACK_FPS];
	}
	// Exception
	if ( mFPS_*ASYNC_TOLERANCE_MS/1000 < mFrameCount_collisionOnServer )
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
			mHasTetriminos = false;
		}

		if ( std::optional<std::string> curTetType(mNet->getByTag(TAG_MY_CURRENT_TETRIMINO,
																::scene::online::Online::Option::SPECIFIED_SIZE,
																  sizeof(::model::tetrimino::Type)));
			std::nullopt != curTetType )
		{
			const ::model::tetrimino::Type type =
				(::model::tetrimino::Type)::ntohl(*(u_long*)curTetType.value().data());
			mCurrentTetrimino = ::model::Tetrimino::Spawn( type );
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
				mNextTetriminoPanel.setTetrimino( mNextTetriminoS.front() );
			}
			else if ( false == mHasTetriminos )
			{
				mNextTetriminoPanel.setTetrimino( mNextTetriminoS.front() );
				mHasTetriminos = true;
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
			mFrameCount_clearingVFX = mFPS_;
		}

		hasToRespond = true;
	}
	else if ( true == mHasTetriminoCollidedAlready )
	{
		return false;
	}

	Packet packet;
	if ( false == mHasTetriminos )
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
		const uint32_t inputDelayFPS = mFPS_ * INPUT_DELAY_MS / 1000;
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
			mNextTetriminoPanel.setTetrimino( mNextTetriminoS.front() );
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

void ui::PlayView::draw( )
{
	mStage.draw( );
	if ( 0 < mFrameCount_countdown )
	{
		--mFrameCount_countdown;
		const uint8_t y = mFrameCount_countdown/mFPS_;
		mSprite.setTextureRect( sf::IntRect( 0, 256*y, 256, 256 ) );
		mWindow_->draw( mSprite );
	}

	if ( true == mHasTetriminos )
	{
		mCurrentTetrimino.draw( *mWindow_ );
		mNextTetriminoPanel.draw( );
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