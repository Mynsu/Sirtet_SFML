#include "../../pch.h"
#include "../../ServiceLocatorMirror.h"
#include "AllClear.h"

bool ::scene::inPlay::AllClear::IsInstantiated = false;

scene::inPlay::AllClear::AllClear( sf::RenderWindow& window )
	: mWindow_( window )
{
	ASSERT_TRUE( false == IsInstantiated );

	mDrawingInfo.confettiSpriteClipRow = mDrawingInfo.confettiSpriteClipColumn = 0;
	mSprite.setTexture(mTexture);
	loadResources( );
	if ( false == gService()->sound().playBGM(mBGMPath) )
	{
		gService()->console().printFailure(FailureLevel::WARNING,
										   "File Not Found: "+mBGMPath );
	}

	IsInstantiated = true;
}

scene::inPlay::AllClear::~AllClear( )
{
	IsInstantiated = false;
}

void scene::inPlay::AllClear::loadResources( )
{
	mDrawingInfo.confettiSpriteClipRowMax_ = mDrawingInfo.confettiSpriteClipColumnMax_ = 8;
	mDrawingInfo.confettiRelativePlaySpeed = 1.0f;
	mDrawingInfo.confettiSpriteClipSize.x = 512;
	mDrawingInfo.confettiSpriteClipSize.y = 512;
	sf::Vector2f confettiDestination( 200.f, 200.f );
	std::string confettiSpritePath( "Images/Confetti.png" );
	mBGMPath = "Sounds/allLevelsCleared.wav";

	lua_State* lua = luaL_newstate();
	const std::string scriptPath( "Scripts/AllClear.lua" );
	if ( true == luaL_dofile(lua, scriptPath.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL,
										   "File Not Found: "+scriptPath );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		std::string tableName( "Confetti" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
												   tableName, scriptPath );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				confettiSpritePath = lua_tostring(lua, TOP_IDX);
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

			field = "clipWidth";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.confettiSpriteClipSize.x = (int32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "clipHeight";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.confettiSpriteClipSize.y = (int32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "x";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				confettiDestination.x = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				confettiDestination.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "speed";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.confettiRelativePlaySpeed = (float)lua_tonumber(lua, TOP_IDX);
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

		tableName = "Sound";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPath );
		}
		else
		{
			std::string innerTableName( "BGM" );
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													   tableName+':'+innerTableName, scriptPath );
			}
			else
			{
				std::string field( "path" );
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				int type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					mBGMPath = lua_tostring(lua, TOP_IDX);
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
		}
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	if ( false == mTexture.loadFromFile(confettiSpritePath) )
	{
		gService()->console().printFailure( FailureLevel::WARNING,
										   "File Not Found: "+confettiSpritePath );
	}
	const sf::Vector2u size = mTexture.getSize();
	mDrawingInfo.confettiSpriteClipColumnMax_ = size.x/mDrawingInfo.confettiSpriteClipSize.x;
	mDrawingInfo.confettiSpriteClipRowMax_ = size.y/mDrawingInfo.confettiSpriteClipSize.y;
	mSprite.setTextureRect( sf::IntRect(0, 0,
										mDrawingInfo.confettiSpriteClipSize.x,
										mDrawingInfo.confettiSpriteClipSize.y) );
	mSprite.setPosition( confettiDestination );
}

::scene::inPlay::ID scene::inPlay::AllClear::update( std::vector<sf::Event>& )
{
	return ::scene::inPlay::ID::AS_IS;
}

void scene::inPlay::AllClear::draw( )
{
	const Clock::time_point now = Clock::now();
	if ( Clock::duration(std::chrono::milliseconds((int)(30*mDrawingInfo.confettiRelativePlaySpeed)))
						 <= now - mTimePreviousClipDraws )
	{
		mTimePreviousClipDraws = now;
		if ( mDrawingInfo.confettiSpriteClipColumnMax_ <= ++mDrawingInfo.confettiSpriteClipColumn )
		{
			mDrawingInfo.confettiSpriteClipColumn = 0;
			if ( mDrawingInfo.confettiSpriteClipRowMax_ <= ++mDrawingInfo.confettiSpriteClipRow )
			{
				mDrawingInfo.confettiSpriteClipRow = 0;
			}
		}
		mSprite.setTextureRect( sf::IntRect(mDrawingInfo.confettiSpriteClipSize.x*
												mDrawingInfo.confettiSpriteClipColumn,
											mDrawingInfo.confettiSpriteClipSize.y*
												mDrawingInfo.confettiSpriteClipRow,
											mDrawingInfo.confettiSpriteClipSize.x,
											mDrawingInfo.confettiSpriteClipSize.y) );
	}
	mWindow_.draw( mSprite );
}
