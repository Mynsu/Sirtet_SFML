#include "Playing.h"
#include "../../ServiceLocatorMirror.h"
#include <lua.hpp>
#include <random>

::scene::inPlay::Playing::Playing( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mIsFallingDown( false ), mFrameCount( 0u ), mCellSize( 15u ),
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

void ::scene::inPlay::Playing::update( ::scene::inPlay::IScene** const nextScene, std::queue< sf::Event >& eventQueue )
{
	bool hasCollidedAtThisFrame = false;
	if ( true == mIsFallingDown )
	{
		const uint8_t end = 3u;
		uint8_t i = 0u;
		while ( i != end )
		{
			++mCurrentTetrimino.position.y;
			if ( hasCollidedAtThisFrame = hasCollided( ); true == hasCollidedAtThisFrame )
			{
				// Same as '= !hasCollidedAtThisFrame.'
				mIsFallingDown = false;
				break;
			}
			++i;
		}
		if ( end == i )
		{
			return;
		}
	}
	else if ( 45u < mFrameCount )
	{
		++mCurrentTetrimino.position.y;
		hasCollidedAtThisFrame = hasCollided( );
		mFrameCount = 0u;
	}
	else
	{
		uint8_t minX = GRID_WIDTH, maxX = 0u;
		for ( uint8_t i = 0u; i != BLOCKS_A_TETRIMINO*BLOCKS_A_TETRIMINO; ++i )
		{
			if ( ( mCurrentTetrimino.rotations[ mCurrentTetrimino.rotationID ] >> i ) & 1u )
			{
				const uint8_t x = i % 4u;
				if ( x < minX )
				{
					minX = x;
				}
				else if ( maxX < x )
				{
					maxX = x;
				}
			}
		}

		while ( false == eventQueue.empty( ) )
		{
			const sf::Event event( eventQueue.front( ) ); //궁금: 복사가 나으려나, 레퍼런스가 나으려나? 실험해보자.
			if ( sf::Event::KeyPressed == event.type )
			{
				switch ( event.key.code )
				{
					case sf::Keyboard::Space:
						mIsFallingDown = true;
						// NOTE: Don't 'return', or it can't come out of the infinite loop.
						///return;
						[[ fallthrough ]];
					case sf::Keyboard::Down:
						++mCurrentTetrimino.position.y;
						hasCollidedAtThisFrame = hasCollided( );
						break;
					case sf::Keyboard::Left:
						if ( 0 < mCurrentTetrimino.position.x + minX )
						{
							--mCurrentTetrimino.position.x;
						}
						break;
					case sf::Keyboard::Right:
						if ( mCurrentTetrimino.position.x + maxX < GRID_WIDTH - 1 )
						{
							++mCurrentTetrimino.position.x;
						}
						break;
					default:
						break;
				}
			}
			eventQueue.pop( );
		}

		//if ( true == sf::Keyboard::isKeyPressed( sf::Keyboard::LShift ) )
		//{
		//	const uint8_t nextRotID = (mCurrentTetrimino.rotationID+1u) % 4u;
		//	for ( uint8_t i = 0u; i != 16; ++i )
		//	{
		//		if ( (mCurrentTetrimino.rotations[nextRotID]>>i) & 1u )
		//		{
		//			const uint8_t x = mCurrentTetrimino.position.x + i%4u;

		//		}
		//	}
		//	/*++mCurrentTetrimino.rotationID;
		//	if ( ROTATION_NUM == mCurrentTetrimino.rotationID )
		//	{
		//		mCurrentTetrimino.rotationID = 0u;
		//	}*/
		//}
	}

	if ( true == hasCollidedAtThisFrame )
	{
		for ( uint8_t i = 0u; i != BLOCKS_A_TETRIMINO*BLOCKS_A_TETRIMINO; ++i )
		{
			if ( (mCurrentTetrimino.rotations[mCurrentTetrimino.rotationID]>>i) & 1u )
			{
				const uint8_t x = mCurrentTetrimino.position.x + i%BLOCKS_A_TETRIMINO;
				uint8_t y = mCurrentTetrimino.position.y + i/BLOCKS_A_TETRIMINO;
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

::scene::inPlay::Tetrimino scene::inPlay::Playing::spawn( ) //TODO
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
	mFrameCount = 0u;

	return retVal;
}

bool scene::inPlay::Playing::hasCollided( )
{
	bool retVal = false;
	for ( int8_t i = BLOCKS_A_TETRIMINO*BLOCKS_A_TETRIMINO - 1; i != -1; --i )
	{
		if ( (mCurrentTetrimino.rotations[mCurrentTetrimino.rotationID]>>i) & 1u )
		{
			const uint8_t x = mCurrentTetrimino.position.x + i%BLOCKS_A_TETRIMINO;
			const uint8_t y = mCurrentTetrimino.position.y + i/BLOCKS_A_TETRIMINO;
			if ( GRID_HEIGHT == y )
			{
				retVal = true;
				break;
			}
			else if ( true == mStage[ y ][ x ].blocked )
			{
				retVal = true;
				break;
			}
		}
	}

	return retVal;
}
