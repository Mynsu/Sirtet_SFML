#include "Playing.h"
#include "../../ServiceLocatorMirror.h"
#include <lua.hpp>

::scene::inPlay::Playing::Playing( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mLineCleared( 0u ), mFrameCount0_( 0 ), mFrameCount1_( 0 ), mFrameCount2_( 0 ), mTempo( 0.75f ),
	mWindow_( window ), mBackgroundRect_( static_cast<sf::RectangleShape&>(shapeOrSprite) ),
	mCurrentTetrimino( ::model::Tetrimino::Spawn( ) ), mPlayerStage( window ), mVfxCombo( window )
{
	const sf::Color cyan( 0x29cdb5fau );
	mBackgroundRect_.setFillColor( cyan );

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
	sf::Vector2i vfxWH( 256, 256 );
	sf::Vector2f nextPanelPos( 500.f, 100.f );
	bool isDefault = true;

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/Playing.lua" );
	if ( true == luaL_dofile( lua, scriptPathNName.data( ) ) )
	{
		// File Not Found Exception
		ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, "File Not Found: " + scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		const std::string tableName0( "PlayerPanel" );
		lua_getglobal( lua, tableName0.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0, scriptPathNName );
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
				panelPos.x = static_cast<float>(lua_tonumber( lua, TOP_IDX ));
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0+":"+field0, scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "y";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				panelPos.y = static_cast<float>(lua_tonumber( lua, TOP_IDX ));
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0+":"+field1, scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "cellSize";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				cellSize = static_cast<float>(lua_tonumber( lua, TOP_IDX ));
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0+":"+field2, scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		const std::string tableName1( "VfxCombo" );
		lua_getglobal( lua, tableName1.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName1, scriptPathNName );
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
					ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::FILE_NOT_FOUND,
																		tableName1+":"+field0, scriptPathNName );
				}
				else
				{
					isDefault = false;
				}
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0+":"+field0, scriptPathNName );
			}
			lua_pop( lua, 1 );


			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				vfxWH.x = static_cast<int>(lua_tointeger( lua, TOP_IDX ));
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName1+":"+field1, scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "height";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				vfxWH.y = static_cast<int>(lua_tointeger( lua, TOP_IDX ));
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName1+":"+field1, scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		const std::string tableName2( "NextTetriminoPanel" );
		lua_getglobal( lua, tableName2.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName2, scriptPathNName );
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
				nextPanelPos.x = static_cast<float>(lua_tonumber( lua, TOP_IDX ));
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName2+":"+field0, scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "y";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextPanelPos.y = static_cast<float>(lua_tonumber( lua, TOP_IDX ));
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName2+":"+field1, scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	if ( true == isDefault )
	{
		const std::string defaultFilePathNName( "Vfxs/Combo.png" );
		if ( false == mVfxCombo.loadResources( defaultFilePathNName ) )
		{
			// Exception: When there's not even the default file,
			ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, "File Not Found: " + defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}
	mPlayerStage.setPosition( panelPos );
	mCurrentTetrimino.setOrigin( panelPos );
	mVfxCombo.setOrigin( panelPos, cellSize, vfxWH );
	mPlayerStage.setSize( cellSize );
	mCurrentTetrimino.setSize( cellSize );
	mNextTetriminoPanel.setPosition( nextPanelPos );
	mNextTetriminoPanelPosition_ = nextPanelPos;
	mNextTetriminoPanel.setSize(
		sf::Vector2f(::model::tetrimino::BLOCKS_A_TETRIMINO+2,::model::tetrimino::BLOCKS_A_TETRIMINO+2)*cellSize );
	mMargin_.x = cellSize;
	mMargin_.y = cellSize;
	mNextTetriminoBlock_.setSize( sf::Vector2f( cellSize, cellSize ) );
	mCellSize_ = cellSize;
}

void ::scene::inPlay::Playing::update( ::scene::inPlay::IScene** const nextScene, std::queue< sf::Event >& eventQueue )
{
	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
	const int32_t fps = ::ServiceLocatorMirror::Vault( ).at( HK_FORE_FPS );
	bool hasCollidedAtThisFrame = false;
	if ( true == mCurrentTetrimino.isFallingDown( ) )
	{
		for ( uint8_t i = 0u; i != 3u; ++i )
		{
			if ( hasCollidedAtThisFrame = mCurrentTetrimino.down( mPlayerStage.grid( ) ); true == hasCollidedAtThisFrame )
			{
				mCurrentTetrimino.fallDown( false );
				// NOTE: Break the loop and stop stuff in the 1st if-scope immediately.
				goto last;
			}
		}
		return;
	}
	else
	{
		while ( false == eventQueue.empty( ) )
		{
			const sf::Event event( eventQueue.front( ) ); //궁금: 복사가 나으려나, 레퍼런스가 나으려나? 실험해보자.
			if ( sf::Event::KeyPressed == event.type )
			{
				switch ( event.key.code )
				{
					case sf::Keyboard::Space:
						mCurrentTetrimino.fallDown( );
						// NOTE: Don't 'return', or it can't come out of the infinite loop.
						///return;
						[[ fallthrough ]];
					case sf::Keyboard::Down:
						hasCollidedAtThisFrame = mCurrentTetrimino.down( mPlayerStage.grid( ) );
						mFrameCount0_ = 0;
						break;
					case sf::Keyboard::Left:
						mCurrentTetrimino.tryMoveLeft( mPlayerStage.grid( ) );
						break;
					case sf::Keyboard::Right:
						mCurrentTetrimino.tryMoveRight( mPlayerStage.grid( ) );
						break;
					case sf::Keyboard::LShift:
						[[ fallthrough ]];
					case sf::Keyboard::Up:
						mCurrentTetrimino.tryRotate( mPlayerStage.grid( ) );
						break;
					default:
						break;
				}
			}
			eventQueue.pop( );
		}
	}
	
	if ( static_cast<int>(fps*mTempo) < mFrameCount0_ )
	{
		hasCollidedAtThisFrame = mCurrentTetrimino.down( mPlayerStage.grid( ) );
		mFrameCount0_ = 0;
	}

	last:
	if ( true == hasCollidedAtThisFrame )
	{
		const ::model::LocalSpace blocks = mCurrentTetrimino.blocks( );
		const uint8_t area = ::model::tetrimino::BLOCKS_A_TETRIMINO*::model::tetrimino::BLOCKS_A_TETRIMINO;
		for ( uint8_t i = 0u; i != area; ++i )
		{
			if ( blocks & (0x1u<<(area-i-1u)) )
			{
				const sf::Vector2<int8_t> pos = mCurrentTetrimino.position( );
				mPlayerStage.lock( pos.x + i%model::tetrimino::BLOCKS_A_TETRIMINO,
								   pos.y + i/model::tetrimino::BLOCKS_A_TETRIMINO - 1,
								   mCurrentTetrimino.color( ) );
			}
		}
		mCurrentTetrimino = mNextTetriminos.front( );
		mCurrentTetrimino.setOrigin( mPlayerStage.position( ) );
		mCurrentTetrimino.setSize( mCellSize_ );
		mNextTetriminos.pop( );
		mNextTetriminos.emplace( ::model::Tetrimino::Spawn( ) );
		mNextTetriminoBlock_.setFillColor( mNextTetriminos.front( ).color( ) );
		mFrameCount0_ = 0;
	}

	if ( static_cast<int>(fps*0.1f) < mFrameCount1_ )
	{
		const uint8_t lineCleared = mPlayerStage.clearLine( );
		mFrameCount1_ = 0;
		if ( 0 != lineCleared )
		{
			mLineCleared = lineCleared;
			mTempo -= 0.02f;
			++mFrameCount2_;
		}
	}
	
	//TODO: 대기하고 있는 다음, 다다음 테트리미노 보여주기
	//궁금: 숨기기, 반대로 움직이기, 일렁이기, 대기열 가리기 같은 아이템 구현하는 게 과연 좋을까?

	//TODO: 게임오버 씬 바꾸기
}

void ::scene::inPlay::Playing::draw( )
{
	mWindow_.draw( mBackgroundRect_ ); //TODO: Z 버퍼로 컬링해서 부하를 줄여볼까?
	mPlayerStage.draw( );
	mCurrentTetrimino.draw( mWindow_ );
	if ( 0 != mFrameCount2_ )
	{
		mVfxCombo.draw( mLineCleared );
		++mFrameCount2_;
	}
	mWindow_.draw( mNextTetriminoPanel );
	const ::model::Tetrimino& nextTet = mNextTetriminos.front( );
	const ::model::LocalSpace nextTetBlocks = nextTet.blocks( );
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

	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
	const int32_t fps = ::ServiceLocatorMirror::Vault( )[ HK_FORE_FPS ];
	if ( fps <= mFrameCount2_ )
	{
		mFrameCount2_ = 0;
	}
	++mFrameCount0_;
	++mFrameCount1_;
}