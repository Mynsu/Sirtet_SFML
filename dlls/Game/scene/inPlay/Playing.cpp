#include "Playing.h"
#include "../../ServiceLocatorMirror.h"
#include <lua.hpp>
#include <random>

#define GRID_WIDTH 10
#define GRID_HEIGHT 20

::scene::inPlay::Playing::Playing( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mFrameCount( 0u ), mCellSize( 15u ),
	mWindow( window ), mBackgroundRect( static_cast< sf::RectangleShape& >( shapeOrSprite ) )
{
	// Cyan
	const sf::Color cyan( 0x29cdb5fau );
	mBackgroundRect.setFillColor( cyan );

	loadResources( );

	mCurrentTetrimino = spawn( );
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
		mCellSize = static_cast< float >( lua_tonumber( lua, -1 ) );
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	mPlayerPanel.setPosition( panelPos );
	mPlayerPanel.setSize( sf::Vector2f( mCellSize*GRID_WIDTH, mCellSize*GRID_HEIGHT ) );
	mPlayerPanel.setFillColor( sf::Color::Black );
}

void ::scene::inPlay::Playing::update( ::scene::inPlay::IScene** const nextScene )
{
	++mCurrentTetrimino.position.y;

	if 
}

void ::scene::inPlay::Playing::draw( )
{
	mWindow.draw( mBackgroundRect ); //TODO: Z 버퍼로 컬링해서 부하를 줄여볼까?
	mWindow.draw( mPlayerPanel );

	sf::RectangleShape rect;
	rect.setSize( sf::Vector2f( mCellSize, mCellSize ) );
	rect.setFillColor( sf::Color::White );
	rect.setOutlineThickness( 1.f );
	rect.setOutlineColor( sf::Color::Red );
	const sf::Vector2f origin( mPlayerPanel.getPosition( ) + sf::Vector2f( mCurrentTetrimino.position )*mCellSize );
	for ( uint8_t i = 0; i != 16; ++i ) //TODO: 16을
	{
		if ( (mCurrentTetrimino.rotation[ mCurrentTetrimino.rotationID ]>>i) & 1 ) //궁금: msb 0으로 채워질까 1일까?
		{
			const sf::Vector2u position( i % 4, i / 4 );
			rect.setPosition( origin + sf::Vector2f( position )*mCellSize );
			mWindow.draw( rect );
		}
	}

	++mFrameCount;
}

::scene::inPlay::Tetrimino scene::inPlay::Playing::spawn( )
{
	std::random_device rD;
	std::minstd_rand rE( rD( ) ); //궁금: 레퍼런스 문서 보자.
	std::uniform_int_distribution shapeDist( 0, static_cast< uint8_t >( TetriminoShape::NONE_MAX ) - 1 ); //궁금: 얘도.
	TetriminoShape shape = static_cast< TetriminoShape >( shapeDist( rE ) );
	Tetrimino retVal;
	switch ( shape )
	{
		case TetriminoShape::I:
			retVal.rotation[ 0 ] = 0b0100'0100'0100'0100;
			retVal.rotation[ 1 ] = 0b0000'0000'1111'0000;
			retVal.rotation[ 2 ] = retVal.rotation[ 0 ];//TODO
			retVal.rotation[ 3 ] = retVal.rotation[ 1 ];//TODO
			break;
		case TetriminoShape::J:
			retVal.rotation[ 0 ] = 0b0010'0010'0110'0000;
			retVal.rotation[ 1 ] = 0b0000'0100'0111'0000;
			retVal.rotation[ 2 ] = 0b0000'0110'0100'0100;
			retVal.rotation[ 3 ] = 0b0000'1110'0010'0000;
			break;
		case TetriminoShape::L:
			retVal.rotation[ 0 ] = 0b0100'0100'0110'0000;
			retVal.rotation[ 1 ] = 0b0000'0010'1110'0000;
			retVal.rotation[ 2 ] = 0b0000'0110'0010'0010;
			retVal.rotation[ 3 ] = 0b0000'0111'0100'0000;
			break;
		case TetriminoShape::N:
			retVal.rotation[ 0 ] = 0b0010'0110'0100'0000;
			retVal.rotation[ 1 ] = 0b0000'0110'0011'0000;
			retVal.rotation[ 2 ] = retVal.rotation[ 0 ];//TODO
			retVal.rotation[ 3 ] = retVal.rotation[ 1 ];//TODO
			break;
		case TetriminoShape::S:
			retVal.rotation[ 0 ] = 0b0100'0110'0010'0000;
			retVal.rotation[ 1 ] = 0b0000'0011'0110'0000;
			retVal.rotation[ 2 ] = retVal.rotation[ 0 ];//TODO
			retVal.rotation[ 3 ] = retVal.rotation[ 1 ];//TODO
			break;
		case TetriminoShape::T:
			retVal.rotation[ 0 ] = 0b0000'0100'1110'0000;
			retVal.rotation[ 1 ] = 0b0100'0110'0100'0000;
			retVal.rotation[ 2 ] = 0b0000'0111'0010'0000;
			retVal.rotation[ 3 ] = 0b0000'0010'0110'0010;
			break;
		case TetriminoShape::O:
			retVal.rotation[ 0 ] = 0b0000'0110'0110'0000;
			retVal.rotation[ 1 ] = retVal.rotation[ 0 ];//TODO
			retVal.rotation[ 2 ] = retVal.rotation[ 0 ];//TODO
			retVal.rotation[ 3 ] = retVal.rotation[ 0 ];//TODO
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
	retVal.position.x = GRID_WIDTH / 2 - 1;
	retVal.position.y = 0u;
	std::uniform_int_distribution rotDist( 0, 3 ); //TODO: 3을
	retVal.rotationID = rotDist( rE );

	return retVal;
}
