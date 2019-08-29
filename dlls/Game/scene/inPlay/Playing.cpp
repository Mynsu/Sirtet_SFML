#include "Playing.h"
#include "../../ServiceLocatorMirror.h"
#include <lua.hpp>

::scene::inPlay::Playing::Playing( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mLineCleared( 0u ), mFrameCount0( 0 ), mFrameCount1( 0 ), mFrameCount2( 0 ),
	mWindow_( window ), mBackgroundRect_( static_cast< sf::RectangleShape& >( shapeOrSprite ) ),
	mCurrentTetrimino( ::model::Tetrimino::Spawn( ) ),
	mPlayerStage( window ), mVfxCombo( window )
{
	// Cyan
	const sf::Color cyan( 0x29cdb5fau );
	mBackgroundRect_.setFillColor( cyan );

	loadResources( );
}

void ::scene::inPlay::Playing::loadResources( )
{
	sf::Vector2f panelPos( 130.f, 0.f );
	float cellSize = 30.f;
	sf::Vector2i vfxWH( 256, 256 );
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
				if ( false == mVfxCombo.loadResources( lua_tostring( lua, TOP_IDX ) ) )
				{
					// File Not Found Exception
					ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::FILE_NOT_FOUND, tableName1+":"+field0, scriptPathNName );
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
				vfxWH.x = static_cast< int >( lua_tointeger(lua,TOP_IDX) );
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
				vfxWH.y = static_cast< int >( lua_tointeger(lua, TOP_IDX) );
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName1+":"+field1, scriptPathNName );
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
						mFrameCount0 = 0;
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
	
	if ( static_cast<int>(fps*0.75f) < mFrameCount0 )
	{
		hasCollidedAtThisFrame = mCurrentTetrimino.down( mPlayerStage.grid( ) );
		mFrameCount0 = 0;
	}

	last:
	if ( true == hasCollidedAtThisFrame )
	{
		const ::model::LocalSpace blocks = mCurrentTetrimino.blocks( );
		for ( uint8_t i = 0u; i != ::model::tetrimino::BLOCKS_A_TETRIMINO*::model::tetrimino::BLOCKS_A_TETRIMINO; ++i )
		{
			if ( (blocks>>i) & 1u )
			{
				const sf::Vector2<int8_t> pos = mCurrentTetrimino.position( );
				mPlayerStage.lock( pos.x + i%model::tetrimino::BLOCKS_A_TETRIMINO,
								   pos.y + i/model::tetrimino::BLOCKS_A_TETRIMINO - 1,
								   mCurrentTetrimino.color( ) );
			}
		}
		mCurrentTetrimino = ::model::Tetrimino::Spawn( );
		mFrameCount0 = 0;
	}

	if ( static_cast<int>(fps*0.1f) < mFrameCount1 )
	{
		const uint8_t lineCleared = mPlayerStage.clearLine( );
		mFrameCount1 = 0;
		if ( 0 != lineCleared )
		{
			mLineCleared = lineCleared;
			++mFrameCount2;
		}
	}
	
	//TODO: 템포 점점 빠르게
	//TODO: 대기하고 있는 다음, 다다음 테트리미노 보여주기
	//궁금: 숨기기, 반대로 움직이기, 일렁이기, 대기열 가리기 같은 아이템 구현하는 게 과연 좋을까?

	//TODO: 게임오버 씬 바꾸기
}

void ::scene::inPlay::Playing::draw( )
{
	mWindow_.draw( mBackgroundRect_ ); //TODO: Z 버퍼로 컬링해서 부하를 줄여볼까?
	mPlayerStage.draw( );
	mCurrentTetrimino.draw( mWindow_ );
	if ( 0 != mFrameCount2 )
	{
		mVfxCombo.draw( mLineCleared );
		++mFrameCount2;
	}
	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
	const int32_t fps = ::ServiceLocatorMirror::Vault( )[ HK_FORE_FPS ];
	if ( fps <= mFrameCount2 )
	{
		mFrameCount2 = 0;
	}

	++mFrameCount0;
	++mFrameCount1;
}