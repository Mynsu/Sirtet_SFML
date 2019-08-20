#include "Playing.h"
#include "../../ServiceLocatorMirror.h"
#include <lua.hpp>
#include <random>

//#define GRID_WIDTH 10
//#define GRID_HEIGHT 20

//constexpr uint8_t scene::inPlay::GRID_WIDTH = 10;
//constexpr uint8_t scene::inPlay::GRID_HEIGHT = 20;

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
	// TODO: 방향키에 따라

	if ( 30u == mFrameCount )
	{
		++mCurrentTetrimino.position.y;
		mFrameCount = 0u;
	}

	bool isLanding = false;
	for ( int8_t i = BLOCKS_A_TETRIMINO*BLOCKS_A_TETRIMINO-1; i != -1; --i )
	{
		if ( (mCurrentTetrimino.rotations[mCurrentTetrimino.rotationID]>>i) & 1 )
		{
			const uint8_t x = mCurrentTetrimino.position.x + i%BLOCKS_A_TETRIMINO;//궁금: 캐스팅 잘 되려나?
			const uint8_t y = mCurrentTetrimino.position.y + i/BLOCKS_A_TETRIMINO;//궁금: 캐스팅 잘 되려나?
			if ( GRID_HEIGHT == y || true == mStage[ y ][ x ].blocked )
			{
				isLanding = true;
				break;
			}
		}
	}

	if ( true == isLanding )
	{
		for ( int8_t i = BLOCKS_A_TETRIMINO*BLOCKS_A_TETRIMINO-1; i != -1; --i )
		{
			if ( (mCurrentTetrimino.rotations[mCurrentTetrimino.rotationID]>>i) & 1 )
			{
				const uint8_t x = mCurrentTetrimino.position.x + i%BLOCKS_A_TETRIMINO;//궁금: 캐스팅 잘 되려나?
				uint8_t y = mCurrentTetrimino.position.y + i/BLOCKS_A_TETRIMINO;//궁금: 캐스팅 잘 되려나?
				mStage[ --y ][ x ].blocked = true;
				mStage[ y ][ x ].color = mCurrentTetrimino.color;
			}
		}
		mCurrentTetrimino = spawn( );
	}

	//TODO: 게임오버 씬 바꾸기
}

void ::scene::inPlay::Playing::draw( )
{
	mWindow.draw( mBackgroundRect ); //TODO: Z 버퍼로 컬링해서 부하를 줄여볼까?
	mWindow.draw( mPlayerPanel );

	sf::RectangleShape rect;
	rect.setSize( sf::Vector2f( mCellSize, mCellSize ) );
	rect.setOutlineThickness( 1.f );
	rect.setOutlineColor( sf::Color::Black );
	sf::Vector2f origin( mPlayerPanel.getPosition( ) );
	for ( uint8_t i = 0u; i != GRID_HEIGHT; ++i )
	{
		for ( uint8_t k = 0u; k != GRID_WIDTH; ++k )
		{
			if ( true == mStage[ i ][ k ].blocked )
			{
				rect.setFillColor( mStage[ i ][ k ].color );
				rect.setPosition( origin + sf::Vector2f( static_cast<float>(k), static_cast<float>(i) )*mCellSize );
				mWindow.draw( rect );
			}
		}
	}

	rect.setFillColor( mCurrentTetrimino.color );
	origin += sf::Vector2f( mCurrentTetrimino.position )*mCellSize;
	for ( uint8_t i = 0; i != BLOCKS_A_TETRIMINO*BLOCKS_A_TETRIMINO; ++i )
	{
		if ( (mCurrentTetrimino.rotations[ mCurrentTetrimino.rotationID ]>>i) & 1u ) //궁금: msb 0으로 채워질까 1일까?
		{
			const sf::Vector2< uint8_t > position( i % BLOCKS_A_TETRIMINO, i / BLOCKS_A_TETRIMINO );
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
	std::uniform_int_distribution shapeDist( 0, static_cast< uint8_t >( TetriminoShape::NONE_MAX )-1 ); //궁금: 얘도.
	std::uniform_int_distribution rotDist( 0, ROTATION_NUM-1 );
	TetriminoShape shape = static_cast< TetriminoShape >( shapeDist( rE ) );
	Tetrimino retVal;
	switch ( shape )
	{
		case TetriminoShape::I:
			retVal.rotations[ 0 ] = 0b0100'0100'0100'0100;
			retVal.rotations[ 1 ] = 0b0000'0000'1111'0000;
			retVal.rotations[ 2 ] = retVal.rotations[ 0 ];//TODO
			retVal.rotations[ 3 ] = retVal.rotations[ 1 ];//TODO
			retVal.rotationID = rotDist( rE );
			retVal.color = sf::Color::Cyan;
			break;
		case TetriminoShape::J:
			retVal.rotations[ 0 ] = 0b0010'0010'0110'0000;
			retVal.rotations[ 1 ] = 0b0000'0100'0111'0000;
			retVal.rotations[ 2 ] = 0b0000'0110'0100'0100;
			retVal.rotations[ 3 ] = 0b0000'1110'0010'0000;
			retVal.rotationID = 1;
			retVal.color = sf::Color::Blue;
			break;
		case TetriminoShape::L:
			retVal.rotations[ 0 ] = 0b0100'0100'0110'0000;
			retVal.rotations[ 1 ] = 0b0000'0010'1110'0000;
			retVal.rotations[ 2 ] = 0b0000'0110'0010'0010;
			retVal.rotations[ 3 ] = 0b0000'0111'0100'0000;
			retVal.rotationID = 1;
			// Orange
			retVal.color = sf::Color( 0xff7f00ff );
			break;
		case TetriminoShape::N:
			retVal.rotations[ 0 ] = 0b0010'0110'0100'0000;
			retVal.rotations[ 1 ] = 0b0000'0110'0011'0000;
			retVal.rotations[ 2 ] = retVal.rotations[ 0 ];//TODO
			retVal.rotations[ 3 ] = retVal.rotations[ 1 ];//TODO
			retVal.rotationID = rotDist( rE );
			retVal.color = sf::Color::Red;
			break;
		case TetriminoShape::S:
			retVal.rotations[ 0 ] = 0b0100'0110'0010'0000;
			retVal.rotations[ 1 ] = 0b0000'0011'0110'0000;
			retVal.rotations[ 2 ] = retVal.rotations[ 0 ];//TODO
			retVal.rotations[ 3 ] = retVal.rotations[ 1 ];//TODO
			retVal.rotationID = rotDist( rE );
			retVal.color = sf::Color::Green;
			break;
		case TetriminoShape::T:
			retVal.rotations[ 0 ] = 0b0000'0100'1110'0000;
			retVal.rotations[ 1 ] = 0b0100'0110'0100'0000;
			retVal.rotations[ 2 ] = 0b0000'0111'0010'0000;
			retVal.rotations[ 3 ] = 0b0000'0010'0110'0010;
			retVal.rotationID = 0;
			// Purple - Old Citadel
			retVal.color = sf::Color( 0x562f72ff );
			break;
		case TetriminoShape::O:
			retVal.rotations[ 0 ] = 0b0000'0110'0110'0000;
			retVal.rotations[ 1 ] = retVal.rotations[ 0 ];//TODO
			retVal.rotations[ 2 ] = retVal.rotations[ 0 ];//TODO
			retVal.rotations[ 3 ] = retVal.rotations[ 0 ];//TODO
			retVal.rotationID = rotDist( rE );
			retVal.color = sf::Color::Yellow;
			break;
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
	retVal.position.x = GRID_WIDTH/2 - 1u;
	retVal.position.y = 0u;

	return retVal;
}
