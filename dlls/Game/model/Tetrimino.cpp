#include "../pch.h"
#include "Tetrimino.h"
#include <Lib/Common.h>
#include "../ServiceLocatorMirror.h"

sf::Vector2<int8_t> model::Tetrimino::Test[(int)::model::tetrimino::Rotation::NONE_MAX][4] =
{ 
	{ {1,0}, {1,-1}, {0,2}, {1,2} },
	{ {-1,0}, {-1,1}, {0,-2}, {-1,-2} },
	{ {-1,0}, {-1,-1}, {0,2}, {-1,2} },
	{ {1,0}, {1,1}, {0,-2}, {1,-2} },
};

sf::Color model::Tetrimino::OutlineColor( 0xffffffff );

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

void model::Tetrimino::LoadResources( )
{
	lua_State* lua = luaL_newstate();
	const std::string scriptPath( "Scripts/Tetrimino.lua" );
	if ( true == luaL_dofile(lua, scriptPath.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL,
										   "File Not Found: "+scriptPath );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		std::string tableName( "Color" );
		lua_getglobal( lua, tableName.data( ) );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
												   tableName, scriptPath );
		}
		else
		{
			std::string field( "I" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::I] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "J";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::J] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "L";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::L] = sf::Color((uint32_t)lua_tonumber(lua, TOP_IDX));
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "N";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::N] = sf::Color((uint32_t)lua_tonumber(lua, TOP_IDX));
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "S";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::S] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "T";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::T] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "O";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				Colors[(int)Type::O] = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		std::string varName( "BlockOutlineColor" );
		lua_getglobal(lua, varName.data() );
		int type = lua_type(lua, TOP_IDX);
		if ( LUA_TNUMBER == type )
		{
			OutlineColor = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
		}
		else if ( LUA_TNIL == type )
		{
			gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
												   varName, scriptPath );
		}
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
												   varName, scriptPath );
		}
		lua_pop( lua, 1 );
	}
	lua_close( lua );
}

::model::Tetrimino model::Tetrimino::Spawn( ::model::tetrimino::Type type )
{
	if ( ::model::tetrimino::Type::NONE_MAX == type )
	{
		std::random_device rD;
		std::minstd_rand rE( rD() );
		std::uniform_int_distribution shapeDist( (int)::model::tetrimino::Type::I,
			(int)::model::tetrimino::Type::NONE_MAX-1 );
		type = (::model::tetrimino::Type)shapeDist(rE);
	}
	::model::Tetrimino retVal;
	retVal.mType = type;
	switch ( type )
	{
		case ::model::tetrimino::Type::I:
			retVal.mPossibleRotations[0] = 0b0100'0100'0100'0100;
			retVal.mPossibleRotations[1] = 0b0000'0000'1111'0000;
			retVal.mPossibleRotations[2] = 0b0010'0010'0010'0010;
			retVal.mPossibleRotations[3] = 0b0000'1111'0000'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)1;
			retVal.setColor( Colors[(int)Type::I], OutlineColor );
			break;
		case ::model::tetrimino::Type::J:
			retVal.mPossibleRotations[0] = 0b1000'1110'0000'0000;
			retVal.mPossibleRotations[1] = 0b0100'0100'1100'0000;
			retVal.mPossibleRotations[2] = 0b0000'1110'0010'0000;
			retVal.mPossibleRotations[3] = 0b0110'0100'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::J], OutlineColor );
			break;
		case ::model::tetrimino::Type::L:
			retVal.mPossibleRotations[0] = 0b0010'1110'0000'0000;
			retVal.mPossibleRotations[1] = 0b1100'0100'0100'0000;
			retVal.mPossibleRotations[2] = 0b0000'1110'1000'0000;
			retVal.mPossibleRotations[3] = 0b0100'0100'0110'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::L], OutlineColor );
			break;
		case ::model::tetrimino::Type::N:
			retVal.mPossibleRotations[0] = 0b1100'0110'0000'0000;
			retVal.mPossibleRotations[1] = 0b0100'1100'1000'0000;
			retVal.mPossibleRotations[2] = 0b0000'1100'0110'0000;
			retVal.mPossibleRotations[3] = 0b0010'0110'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::N], OutlineColor );
			break;
		case ::model::tetrimino::Type::S:
			retVal.mPossibleRotations[0] = 0b0110'1100'0000'0000;
			retVal.mPossibleRotations[1] = 0b0100'0110'0010'0000;
			retVal.mPossibleRotations[2] = 0b0000'0110'1100'0000;
			retVal.mPossibleRotations[3] = 0b1000'1100'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::S], OutlineColor );
			break;
		case ::model::tetrimino::Type::T:
			retVal.mPossibleRotations[0] = 0b0100'1110'0000'0000;
			retVal.mPossibleRotations[1] = 0b0100'1100'0100'0000;
			retVal.mPossibleRotations[2] = 0b0000'1110'0100'0000;
			retVal.mPossibleRotations[3] = 0b0100'0110'0100'0000;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::T], OutlineColor );
			break;
		case ::model::tetrimino::Type::O:
		{
			const uint32_t localspace = 0b0000'0110'0110'0000;
			retVal.mPossibleRotations[0] = localspace;
			retVal.mPossibleRotations[1] = localspace;
			retVal.mPossibleRotations[2] = localspace;
			retVal.mPossibleRotations[3] = localspace;
			retVal.mRotationID = (::model::tetrimino::Rotation)0;
			retVal.setColor( Colors[(int)Type::O], OutlineColor );
			break;
		}
		default:
#ifdef _DEBUG
			__debugbreak( );
#else
			__assume( 0 );
#endif
			break;
	}
	retVal.mBlockShape.setOutlineThickness( -2.f );
	retVal.mPosition.x = ::model::stage::GRID_WIDTH/2 - 1;
	retVal.mPosition.y = 0;

	return retVal;
}

model::Tetrimino::Tetrimino( )
	: mIsHardDropping( false )
{}

model::Tetrimino::Tetrimino( const Tetrimino& arg )
	: mIsHardDropping( false ), mType( arg.mType ),
	mRotationID( arg.mRotationID ), mPosition( arg.mPosition )
{
	mBlockShape.setFillColor( arg.mBlockShape.getFillColor() );
	mBlockShape.setOutlineThickness( arg.mBlockShape.getOutlineThickness() );
	mBlockShape.setOutlineColor( arg.mBlockShape.getOutlineColor() );
	for ( uint8_t i = 0; i != (uint8_t)::model::tetrimino::Rotation::NONE_MAX; ++i )
	{
		mPossibleRotations[i] = arg.mPossibleRotations[i];
	}
}

void model::Tetrimino::operator=( const Tetrimino& arg )
{
	mType = arg.mType;
	mRotationID = arg.mRotationID;
	mPosition = arg.mPosition;
	mBlockShape.setFillColor( arg.mBlockShape.getFillColor() );
	mBlockShape.setOutlineThickness( arg.mBlockShape.getOutlineThickness() );
	mBlockShape.setOutlineColor( arg.mBlockShape.getOutlineColor() );
	for ( uint8_t i = 0; i != (uint8_t)::model::tetrimino::Rotation::NONE_MAX; ++i )
	{
		mPossibleRotations[i] = arg.mPossibleRotations[i];
	}
}

void model::Tetrimino::tryRotate( const ::model::stage::Grid& grid )
{
	// Copying prototype
	Tetrimino afterRot( *this );
	//
	// Prototype's rotation
	//
	uint8_t rotID = (uint8_t)afterRot.mRotationID;
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
	//
	// Moving the prototype around here
	//
	for ( uint8_t i = 0; i != (int)Rot::NONE_MAX; ++i )
	{
		afterRot.mPosition += Test[(int)afterRot.mRotationID][i];
		// When the rotation is possible after moving,
		if ( false == afterRot.hasCollidedWith( grid ) )
		{
			mRotationID = afterRot.mRotationID;
			mPosition = afterRot.mPosition;
			return;
		}
		afterRot.mPosition -= Test[(int)afterRot.mRotationID][i];
	}
}

void model::Tetrimino::land( ::model::stage::Grid& grid )
{
	const ::model::tetrimino::LocalSpace blocks = mPossibleRotations[(int)mRotationID ];
	for ( uint8_t i = 0; i != ::model::tetrimino::LOCAL_SPACE_SIZE; ++i )
	{
		if ( blocks & (0x1<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1)) )
		{
			const uint8_t x = mPosition.x + i%model::tetrimino::BLOCKS_A_TETRIMINO;
			const uint8_t y = mPosition.y + i/model::tetrimino::BLOCKS_A_TETRIMINO;
			ASSERT_TRUE( x < ::model::stage::GRID_WIDTH && y < ::model::stage::GRID_HEIGHT );
			grid[y][x].blocked = true;
			grid[y][x].color = mBlockShape.getFillColor();
		}
	}
}

bool model::Tetrimino::hasCollidedWith( const ::model::stage::Grid& grid ) const
{
	bool retVal = false;
	for ( int8_t i = ::model::tetrimino::LOCAL_SPACE_SIZE-1; i != -1; --i )
	{
		if ( mPossibleRotations[(int)mRotationID]
			& (0x1<<(::model::tetrimino::LOCAL_SPACE_SIZE-i-1)) )
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
			else if ( true == grid[y][x].blocked )
			{
				retVal = true;
				break;
			}
		}
	}
	return retVal;
}