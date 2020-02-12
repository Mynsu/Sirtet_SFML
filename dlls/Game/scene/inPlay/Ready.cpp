#include "../../pch.h"
#include "Ready.h"
#include "../../ServiceLocatorMirror.h"
#include "Playing.h"
#include "../VaultKeyList.h"

::scene::inPlay::Ready::Ready( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mFPS_( 60u ), mFrameCount( mFPS_ * 3 ),
	mWindow_( window ), mBackgroundRect_( static_cast< sf::RectangleShape& >( shapeOrSprite ) ),
	mSpriteClipSize_( 256.f, 256.f )
{
	if ( const auto it = gService()->vault().find(HK_FORE_FPS); gService()->vault().cend() != it )
	{
		mFPS_ = it->second;
		mFrameCount = mFPS_ * 3;
	}

	loadResources( );
}

void ::scene::inPlay::Ready::loadResources( )
{
	uint32_t backgroundColor = 0x29cdb5'7fu;
	std::string countdownSpritePathNName( "Images/Countdown.png" );

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/Ready.lua" );
	if ( true == luaL_dofile(lua, scriptPathNName.data()) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL,
											 "File Not Found: "+scriptPathNName );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		std::string varName( "BackgroundColor" );
		lua_getglobal( lua, varName.data() );
		int type = lua_type(lua, TOP_IDX);
		if ( LUA_TNUMBER == type )
		{
			backgroundColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
		}
		else if ( LUA_TNIL == type )
		{
			gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
												   varName, scriptPathNName );
		}
		else
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 varName, scriptPathNName );
		}
		lua_pop( lua, 1 );

		std::string tableName( "CountdownSprite" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				countdownSpritePathNName = lua_tostring(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "clipWidth";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tointeger(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
															tableName+':'+field, scriptPathNName );
				}
				else
				{
					mSpriteClipSize_.x = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "clipHeight";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
															tableName+':'+field, scriptPathNName );
				}
				else
				{
					mSpriteClipSize_.y = temp;
				}
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	if ( false == mTexture.loadFromFile(countdownSpritePathNName) )
	{
		// Exception: When there's not even the default file,
		gService( )->console( ).printFailure( FailureLevel::FATAL,
												"File Not Found: "+countdownSpritePathNName );
#ifdef _DEBUG
		__debugbreak( );
#endif
	}

	mBackgroundRect_.setFillColor( sf::Color(backgroundColor) );
	mSprite.setTexture( mTexture );
	mSprite.setPosition( (sf::Vector2f(mWindow_.getSize())-mSpriteClipSize_)*0.5f );
}

::scene::inPlay::ID scene::inPlay::Ready::update( std::vector<sf::Event>& )
{
	// NOTE: moved into 'draw( ).'
	///--mFrameCount;

	if ( 0 == mFrameCount )
	{
		return ::scene::inPlay::ID::PLAYING;
	}

	return ::scene::inPlay::ID::AS_IS;
}

void ::scene::inPlay::Ready::draw( )
{
	////
	// Background
	////
	mWindow_.draw( mBackgroundRect_ );

	////
	// Countdown
	////
	const sf::Vector2i cast( mSpriteClipSize_ );
	mSprite.setTextureRect( sf::IntRect( 0, cast.y*( mFrameCount/mFPS_ ), cast.x, cast.y ) );
	mWindow_.draw( mSprite );

	--mFrameCount;
}
