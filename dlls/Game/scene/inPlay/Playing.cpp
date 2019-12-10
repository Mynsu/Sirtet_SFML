#include "../../pch.h"
#include "Playing.h"
#include "../../ServiceLocatorMirror.h"
#include "GameOver.h"
#include "Assertion.h"
#include "../VaultKeyList.h"

const float TEMPO_DIFF_RATIO = 0.02f;
const uint8_t FALLING_DIFF = 3u;
const uint32_t LINE_CLEAR_CHK_INTERVAL_MS = 100;

::scene::inPlay::Playing::Playing( sf::RenderWindow& window,
								   sf::Drawable& shapeOrSprite,
								   const std::unique_ptr<::scene::inPlay::IScene>& overlappedScene )
	: mNumOfLinesCleared( 0u ),
	mFrameCount_fallDown( 0u ), mFrameCount_clearingInterval_( 0u ), mFrameCount_clearingVfx_( 0u ), mFrameCount_gameOver( 0u ),
	mTempo( 0.75f ),
	mWindow_( window ), mBackgroundRect_( (sf::RectangleShape&)shapeOrSprite ),
	mOverlappedScene_( overlappedScene ),
	mCurrentTetrimino( ::model::Tetrimino::Spawn( ) ), mPlayerStage( window ), mVfxCombo( window )
{
//TODO: 스크립트로 옮기기
	const sf::Color CYAN( 0x29cdb5fau );
	mBackgroundRect_.setFillColor( CYAN );

	mNextTetriminos.emplace( ::model::Tetrimino::Spawn( ) );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn( ) );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn( ) );
	mNextTetriminoPanel.setFillColor( sf::Color::Black );
	mNextTetriminoBlock_.setFillColor( mNextTetriminos.front( ).color( ) );
	mNextTetriminoBlock_.setOutlineColor( sf::Color::Black );
	mNextTetriminoBlock_.setOutlineThickness( 1.0f );

	loadResources( );
}

void ::scene::inPlay::Playing::loadResources( )
{
	sf::Vector2f panelPos( 130.f, 0.f );
	float cellSize = 30.f;
	sf::Vector2i vfxSize( 256, 256 );
	sf::Vector2f nextTetPanelPos( 525.f, 70.f );
	bool isDefault = true;

	lua_State* lua = luaL_newstate( );
	const char scriptPathNName[] = "Scripts/Playing.lua";
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
		const std::string tableName0( "PlayerPanel" );
		lua_getglobal( lua, tableName0.data( ) );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			// Type Check Exception
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName0.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "x";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				panelPos.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName0+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "y";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				panelPos.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName0+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "cellSize";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				cellSize = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName0+":"+field2).data(), scriptPathNName );
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
				}
				else
				{
					isDefault = false;
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
				vfxSize.x = (int)lua_tointeger(lua, TOP_IDX);
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
				vfxSize.y = (int)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName1+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		const std::string tableName2( "NextTetriminoPanel" );
		lua_getglobal( lua, tableName2.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName2.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "x";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelPos.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName2+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "y";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelPos.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName2+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	if ( true == isDefault )
	{
		const char defaultFilePathNName[] = "Vfxs/Combo.png";
		if ( false == mVfxCombo.loadResources( defaultFilePathNName ) )
		{
			// Exception: When there's not even the default file,
			gService( )->console( ).printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}
	mPlayerStage.setPosition( panelPos );
	mCurrentTetrimino.setOrigin( panelPos );
	mVfxCombo.setOrigin( panelPos, cellSize, vfxSize );
	mPlayerStage.setSize( cellSize );
	mCurrentTetrimino.setSize( cellSize );
	mNextTetriminoPanel.setPosition( nextTetPanelPos );
	mNextTetriminoPanelPosition_ = nextTetPanelPos;
	mNextTetriminoPanel.setSize(
		sf::Vector2f(::model::tetrimino::BLOCKS_A_TETRIMINO+2,::model::tetrimino::BLOCKS_A_TETRIMINO+2)*cellSize );
	mMargin_.x = cellSize;
	mMargin_.y = cellSize;
	mNextTetriminoBlock_.setSize( sf::Vector2f( cellSize, cellSize ) );
	mCellSize_ = cellSize;
}

::scene::inPlay::ID scene::inPlay::Playing::update( std::list< sf::Event >& eventQueue )
{
	const uint32_t fps = (uint32_t)gService( )->vault( )[HK_FORE_FPS];
	if ( fps < mFrameCount_gameOver )
	{
		return ::scene::inPlay::ID::GAME_OVER;
	}
	else if ( 0u != mFrameCount_gameOver )
	{
		return ::scene::inPlay::ID::AS_IS;
	}

	::scene::inPlay::ID retVal = ::scene::inPlay::ID::AS_IS;
	bool hasCollidedAtThisFrame = false;
	if ( true == mCurrentTetrimino.isFallingDown( ) )
	{
		for ( uint8_t i = 0u; i != FALLING_DIFF; ++i )
		{
			hasCollidedAtThisFrame = mCurrentTetrimino.moveDown(mPlayerStage.cgrid());
			if ( true == hasCollidedAtThisFrame )
			{
				mCurrentTetrimino.fallDown( false );
				// NOTE: Break the loop and stop stuff in the 1st if-scope immediately.
				goto last;
			}
		}
		return ::scene::inPlay::ID::AS_IS;
	}
	else
	{
		for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
		{
			if ( sf::Event::KeyPressed == it->type )
			{
				switch ( it->key.code )
				{
					case sf::Keyboard::Space:
						mCurrentTetrimino.fallDown( );
						// NOTE: Don't 'return', or it can't come out of the infinite loop.
						///return;
						[[ fallthrough ]];
					case sf::Keyboard::Down:
						hasCollidedAtThisFrame = mCurrentTetrimino.moveDown( mPlayerStage.cgrid( ) );
						mFrameCount_fallDown = 0u;
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Left:
						mCurrentTetrimino.tryMoveLeft( mPlayerStage.cgrid( ) );
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Right:
						mCurrentTetrimino.tryMoveRight( mPlayerStage.cgrid( ) );
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::LShift:
						[[ fallthrough ]];
					case sf::Keyboard::Up:
						mCurrentTetrimino.tryRotate( mPlayerStage.cgrid( ) );
						it = eventQueue.erase( it );
						break;
					case sf::Keyboard::Escape:
						if ( nullptr == mOverlappedScene_ ||
							typeid(*mOverlappedScene_) != typeid(::scene::inPlay::Assertion) )
						{
							retVal = ::scene::inPlay::ID::ASSERTION;
							it = eventQueue.erase( it );
						}
						else
						{
							++it;
						}
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
	}
	
	if ( static_cast<uint32_t>(fps*mTempo) < mFrameCount_fallDown )
	{
		hasCollidedAtThisFrame = mCurrentTetrimino.moveDown( mPlayerStage.cgrid( ) );
		mFrameCount_fallDown = 0u;
	}

	last:
	if ( true == hasCollidedAtThisFrame )
	{
		mCurrentTetrimino.land( mPlayerStage.grid() );
		reloadTetrimino( );
	}

	// Check if a row or more have to be cleared,
	// NOTE: It's better to check that every several frames than every frame.
	if ( (uint32_t)fps*LINE_CLEAR_CHK_INTERVAL_MS/1000 < mFrameCount_clearingInterval_ )
	{
		const uint8_t numOfLinesCleared = mPlayerStage.tryClearRow( );
		mFrameCount_clearingInterval_ = 0u;
		if ( 0 != numOfLinesCleared )
		{
			mNumOfLinesCleared = numOfLinesCleared;
			mTempo -= TEMPO_DIFF_RATIO;
			// Making 0 to 1 so as to start the timer.
			++mFrameCount_clearingVfx_;
		}
		if ( true == mPlayerStage.isOver( ) )
		{
			mPlayerStage.blackout( );
			const sf::Color GRAY( 0x808080ff );
			mCurrentTetrimino.setColor( GRAY );
			// Making 0 to 1 so as to start the timer.
			++mFrameCount_gameOver;
		}
	}
	
	//궁금: 숨기기, 반대로 움직이기, 일렁이기, 대기열 가리기 같은 아이템 구현하는 게 과연 좋을까?

	return retVal;
}

void ::scene::inPlay::Playing::draw( )
{
	mWindow_.draw( mBackgroundRect_ ); //TODO: Z 버퍼로 컬링해서 부하를 줄여볼까?
	mPlayerStage.draw( );
	mCurrentTetrimino.draw( mWindow_ );
	if ( 0u != mFrameCount_clearingVfx_ )
	{
		mVfxCombo.draw( mNumOfLinesCleared );
		++mFrameCount_clearingVfx_;
	}
	mWindow_.draw( mNextTetriminoPanel );
	const ::model::Tetrimino& nextTet = mNextTetriminos.front( );
	const ::model::tetrimino::LocalSpace nextTetBlocks = nextTet.blocks( );
	for ( uint8_t i = 0u; i != ::model::tetrimino::BLOCKS_A_TETRIMINO*::model::tetrimino::BLOCKS_A_TETRIMINO; ++i )
	{
		if ( nextTetBlocks & (0x1u<<(::model::tetrimino::BLOCKS_A_TETRIMINO*::model::tetrimino::BLOCKS_A_TETRIMINO-i-1u)) )
		{
			sf::Vector2f localPos( sf::Vector2<uint8_t>(i%model::tetrimino::BLOCKS_A_TETRIMINO,i/model::tetrimino::BLOCKS_A_TETRIMINO) );
			const ::model::tetrimino::Type type = nextTet.type( );
			if ( ::model::tetrimino::Type::O == type )
			{
				mNextTetriminoBlock_.setPosition( mNextTetriminoPanelPosition_ + mMargin_ + localPos*mCellSize_ );
			}
			else if ( ::model::tetrimino::Type::I == type )
			{
				mNextTetriminoBlock_.setPosition( mNextTetriminoPanelPosition_ + mMargin_ - sf::Vector2f(0.f, mCellSize_*0.5f)
												  + localPos*mCellSize_ );
			}
			else
			{
				mNextTetriminoBlock_.setPosition( mNextTetriminoPanelPosition_ + mMargin_*1.5f + sf::Vector2f(0.f,mCellSize_*0.5f)
												  + localPos*mCellSize_);
			}
			mWindow_.draw( mNextTetriminoBlock_ );
		}
	}

	const uint32_t fps = (uint32_t)gService( )->vault( )[ HK_FORE_FPS ];
	if ( fps <= mFrameCount_clearingVfx_ )
	{
		mFrameCount_clearingVfx_ = 0u;
	}
	++mFrameCount_fallDown;
	++mFrameCount_clearingInterval_;
	if ( 0u != mFrameCount_gameOver )
	{
		++mFrameCount_gameOver;
	}
}

void scene::inPlay::Playing::reloadTetrimino()
{
	mCurrentTetrimino = mNextTetriminos.front( );
	mCurrentTetrimino.setOrigin( mPlayerStage.position() );
	mCurrentTetrimino.setSize( mCellSize_ );
	mNextTetriminos.pop( );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminoBlock_.setFillColor( mNextTetriminos.front().color() );
	mFrameCount_fallDown = 0u;
}
