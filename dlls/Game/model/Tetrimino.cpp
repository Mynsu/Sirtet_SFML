#include "../pch.h"
#include "Tetrimino.h"
#include "../ServiceLocatorMirror.h"

sf::Vector2<int8_t> model::Tetrimino::Test[ (int)::model::tetrimino::Rotation::NONE_MAX ][ 4 ] =
	{ 
		{ {1,0}, {1,-1}, {0,2}, {1,2} },
		{ {-1,0}, {-1,1}, {0,-2}, {-1,-2} },
		{ {-1,0}, {-1,-1}, {0,2}, {-1,2} },
		{ {1,0}, {1,1}, {0,-2}, {1,-2} },
	};

using Type = ::model::tetrimino::Type;
sf::Color model::Tetrimino::Colors[(int)Type::NONE_MAX] =
	{
		sf::Color::Cyan,
		sf::Color::Blue,
		sf::Color(0xffa500ff),// Orange
		sf::Color::Red,
		sf::Color::Green,
		sf::Color(0x562f72ff),// Purple - Old Citadel
		sf::Color::Yellow
	};
sf::Color model::Tetrimino::BlockOutlineColor(0x000000'ff);

void model::Tetrimino::LoadResources( )
{
	lua_State* lua = luaL_newstate( );
	const char scriptPathNName[] = "Scripts/Tetrimino.lua";
	if ( true == luaL_dofile(lua, scriptPathNName) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL,
											 std::string("File Not Found: ")+scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		const std::string valName0( "BlockOutlineColor" );
		lua_getglobal( lua, valName0.data() );
		if ( false == lua_isinteger(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 valName0.data( ), scriptPathNName );
		}
		else
		{
			BlockOutlineColor = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
		}
		lua_pop( lua, 1 );

		const std::string tableName0( "Color" );
		lua_getglobal( lua, tableName0.data( ) );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName0.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "I";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::I] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "J";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::J] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "L";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::L] = sf::Color((uint32_t)lua_tonumber(lua, TOP_IDX));
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field2).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field3[ ] = "N";
			lua_pushstring( lua, field3 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::N] = sf::Color((uint32_t)lua_tonumber(lua, TOP_IDX));
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field3).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field4[ ] = "S";
			lua_pushstring( lua, field4 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::S] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field4).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field5[ ] = "T";
			lua_pushstring( lua, field5 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::T] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field5).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field6[ ] = "O";
			lua_pushstring( lua, field6 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::O] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field6).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}
}

::model::Tetrimino model::Tetrimino::Spawn( ::model::tetrimino::Type type )
{
	if ( ::model::tetrimino::Type::NONE_MAX == type )
	{
		std::random_device rD;
		std::minstd_rand rE( rD() ); //±Ã±Ý: ·¹ÆÛ·±½º ¹®¼­ º¸ÀÚ.
		std::uniform_int_distribution shapeDist( (int)::model::tetrimino::Type::I,
												 (int)::model::tetrimino::Type::NONE_MAX-1 ); //±Ã±Ý: ¾êµµ.
		type = (::model::tetrimino::Type)shapeDist(rE);
	}
	::model::Tetrimino retVal;
	retVal.mType = type;
	switch ( type )
	{
		case ::model::tetrimino::Type::I:
			retVal.mPossibleRotations[ 0 ] = 0b0100'0100'0100'0100;
			retVal.mPossibleRotations[ 1 ] = 0b0000'0000'1111'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0010'0010'0010'0010;
			retVal.mPossibleRotations[ 3 ] = 0b0000'1111'0000'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)1;
			retVal.setColor( Colors[(int)Type::I], BlockOutlineColor );
			break;
		case ::model::tetrimino::Type::J:
			retVal.mPossibleRotations[ 0 ] = 0b1000'1110'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b0100'0100'1100'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'1110'0010'0000;
			retVal.mPossibleRotations[ 3 ] = 0b0110'0100'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::J], BlockOutlineColor );
			break;
		case ::model::tetrimino::Type::L:
			retVal.mPossibleRotations[ 0 ] = 0b0010'1110'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b1100'0100'0100'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'1110'1000'0000;
			retVal.mPossibleRotations[ 3 ] = 0b0100'0100'0110'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::L], BlockOutlineColor );
			break;
		case ::model::tetrimino::Type::N:
			retVal.mPossibleRotations[ 0 ] = 0b1100'0110'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b0100'1100'1000'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'1100'0110'0000;
			retVal.mPossibleRotations[ 3 ] = 0b0010'0110'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::N], BlockOutlineColor );
			break;
		case ::model::tetrimino::Type::S:
			retVal.mPossibleRotations[ 0 ] = 0b0110'1100'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b0100'0110'0010'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'0110'1100'0000;
			retVal.mPossibleRotations[ 3 ] = 0b1000'1100'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::S], BlockOutlineColor );
			break;
		case ::model::tetrimino::Type::T:
			retVal.mPossibleRotations[ 0 ] = 0b0100'1110'0000'0000;
			retVal.mPossibleRotations[ 1 ] = 0b0100'1100'0100'0000;
			retVal.mPossibleRotations[ 2 ] = 0b0000'1110'0100'0000;
			retVal.mPossibleRotations[ 3 ] = 0b0100'0110'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::T], BlockOutlineColor );
			break;
		case ::model::tetrimino::Type::O:
		{
			const uint32_t localspace = 0b0000'0110'0110'0000;
			retVal.mPossibleRotations[ 0 ] = localspace;
			retVal.mPossibleRotations[ 1 ] = localspace;
			retVal.mPossibleRotations[ 2 ] = localspace;
			retVal.mPossibleRotations[ 3 ] = localspace;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::O], BlockOutlineColor );
			break;
		}
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
	}
	retVal.mPosition.x = ::model::stage::GRID_WIDTH/2u - 1u;
	retVal.mPosition.y = 0u;

	return retVal;
}

void model::Tetrimino::tryRotate( const ::model::stage::Grid& grid )
{
	// Copying prototype
	Tetrimino afterRot( *this );
	//
	// Prototype's rotation
	//
	uint8_t rotID = static_cast< uint8_t >( afterRot.mRotationID );
	++rotID;
	using Rot = ::model::tetrimino::Rotation;
	afterRot.mRotationID = ( Rot::NONE_MAX == (Rot)rotID )?
		Rot::A: (Rot)rotID;
	// When no collision happens,
	if ( false == afterRot.hasCollidedWith( grid ) )
	{
		mRotationID = afterRot.mRotationID;
		return;
	}
	// When a collision is detected,
	// NOTE: 'else' is omitted.
	///else
	//
	// Moving the prototype around here
	//
	for ( uint8_t i = 0u; i != 4u; ++i )
	{
		afterRot.mPosition += Test[ static_cast<int>(afterRot.mRotationID) ][ i ];
		// When the rotation is possible after moving,
		if ( false == afterRot.hasCollidedWith( grid ) )
		{
			mRotationID = afterRot.mRotationID;
			mPosition = afterRot.mPosition;
			return;
		}
		// When the rotation is impossible,
		// NOTE: 'else' is omitted.
		///else
		afterRot.mPosition -= Test[ static_cast<int>(afterRot.mRotationID) ][ i ];
	}
}

void model::Tetrimino::land( ::model::stage::Grid& grid )
{
	const ::model::tetrimino::LocalSpace blocks = mPossibleRotations[(int)mRotationID ];
	for ( uint8_t i = 0u; i != ::model::tetrimino::LOCAL_SPACE_SIZE; ++i )
	{
		if ( blocks & (0x1u<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1u)) )
		{
			const uint8_t x = mPosition.x + i%model::tetrimino::BLOCKS_A_TETRIMINO;
			const uint8_t y = mPosition.y + i/model::tetrimino::BLOCKS_A_TETRIMINO;
			ASSERT_TRUE( (x<::model::stage::GRID_WIDTH) && (y<::model::stage::GRID_HEIGHT) );
			grid[ y ][ x ].blocked = true;
			grid[ y ][ x ].color = mBlockShape.getFillColor( );
		}
	}
}

bool model::Tetrimino::hasCollidedWith( const ::model::stage::Grid& grid ) const
{
	bool retVal = false;
	for ( int8_t i = ::model::tetrimino::LOCAL_SPACE_SIZE-1; i != -1; --i )
	{
		if ( mPossibleRotations[static_cast<int>(mRotationID)]
			& (0x1u<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1u)) )
		{
			// Coordinate transformation
			const int8_t x = mPosition.x + i%model::tetrimino::BLOCKS_A_TETRIMINO;
			const int8_t y = mPosition.y + i/model::tetrimino::BLOCKS_A_TETRIMINO;
			if ( ::model::stage::GRID_HEIGHT <= y )
			{
				retVal = true;
				break;
			}
			else if ( x < 0 || ::model::stage::GRID_WIDTH <= x )
			{
				retVal = true;
				break;
			}
			else if ( true == grid[ y ][ x ].blocked )
			{
				retVal = true;
				break;
			}
		}
	}
	return retVal;
}