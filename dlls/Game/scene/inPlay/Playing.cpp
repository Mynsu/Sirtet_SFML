#include "Playing.h"
#include "../../ServiceLocatorMirror.h"
#include <lua.hpp>

::scene::inPlay::Playing::Playing( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mFrameCount( 0u ),
	mWindow_( window ), mBackgroundRect_( static_cast< sf::RectangleShape& >( shapeOrSprite ) ),
	mCurrentTetrimino( ::model::Tetrimino::Spawn( ) ),
	mPlayerStage( window )
{
	// Cyan
	const sf::Color cyan( 0x29cdb5fau );
	mBackgroundRect_.setFillColor( cyan );

	loadResources( );
}

void ::scene::inPlay::Playing::loadResources( )
{
	//TODO: array<array<string>>

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/Playing.lua" );
	if ( true == luaL_dofile( lua, scriptPathNName.data( ) ) )
	{
		// File Not Found Exception
		ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, "File Not Found: " + scriptPathNName );
		lua_close( lua );
	}
	luaL_openlibs( lua );
	const std::string tableName( "PlayerPanel" );
	lua_getglobal( lua, tableName.data( ) );
	sf::Vector2f panelPos;
	float cellSize = 30.f;
	// Type Check Exception
	if ( false == lua_istable( lua, -1 ) )
	{
		ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
	}
	else
	{
		lua_pushstring( lua, "x" );
		lua_gettable( lua, 1 );
		panelPos.x = static_cast< float >( lua_tonumber( lua, -1 ) );
		lua_pop( lua, 1 );

		lua_pushstring( lua, "y" );
		lua_gettable( lua, 1 );
		panelPos.y = static_cast< float >( lua_tonumber( lua, -1 ) );
		lua_pop( lua, 1 );

		lua_pushstring( lua, "cellSize" );
		lua_gettable( lua, 1 );
		cellSize = static_cast< float >( lua_tonumber( lua, -1 ) );
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	mPlayerStage.setPosition( panelPos );
	mCurrentTetrimino.setOrigin( panelPos );
	mPlayerStage.setSize( cellSize );
	mCurrentTetrimino.setSize( cellSize );
}

void ::scene::inPlay::Playing::update( ::scene::inPlay::IScene** const nextScene, std::queue< sf::Event >& eventQueue )
{
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
						mFrameCount = 0u;
						break;
					case sf::Keyboard::Left:
						mCurrentTetrimino.tryMoveLeft( mPlayerStage.grid( ) );
						break;
					case sf::Keyboard::Right:
						mCurrentTetrimino.tryMoveRight( mPlayerStage.grid( ) );
						break;
					case sf::Keyboard::LShift:
						mCurrentTetrimino.tryRotate( mPlayerStage.grid( ) );
						break;
					default:
						break;
				}
			}
			eventQueue.pop( );
		}
	}

	if ( 45u < mFrameCount )
	{
		hasCollidedAtThisFrame = mCurrentTetrimino.down( mPlayerStage.grid( ) );
		mFrameCount = 0u;
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
		mFrameCount = 0u;
	}

	//TODO: 라인 클리어

	//TODO: 게임오버 씬 바꾸기
}

void ::scene::inPlay::Playing::draw( )
{
	mWindow_.draw( mBackgroundRect_ ); //TODO: Z 버퍼로 컬링해서 부하를 줄여볼까?
	mPlayerStage.draw( );
	mCurrentTetrimino.draw( mWindow_ );

	++mFrameCount;
}