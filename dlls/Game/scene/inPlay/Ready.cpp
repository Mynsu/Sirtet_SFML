#include "../../pch.h"
#include "Ready.h"
#include "../../ServiceLocatorMirror.h"
#include "Playing.h"

bool ::scene::inPlay::Ready::IsInstantiated = false;

::scene::inPlay::Ready::Ready( const sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mFPS_( 60 ), mFrameCountToStart( mFPS_ * 3 ),
	mBackgroundRect_( (sf::RectangleShape&)shapeOrSprite ),
	mSpriteClipSize( 256.f, 256.f )
{
	ASSERT_TRUE( false == IsInstantiated );

	auto& vault = gService()->vault();
	const auto it = vault.find(HK_FORE_FPS);
	ASSERT_TRUE( vault.cend() != it );
	mFPS_ = it->second;
	mFrameCountToStart = mFPS_ * 3;
	loadResources( window );

	IsInstantiated = true;
}

scene::inPlay::Ready::~Ready()
{
	IsInstantiated = false;
}

void ::scene::inPlay::Ready::loadResources( const sf::RenderWindow& window )
{
	uint32_t backgroundColor = 0x29cdb5'7fu;
	std::string countdownSpritePath( "Images/Countdown.png" );

	lua_State* lua = luaL_newstate();
	const std::string scriptPath( "Scripts/Ready.lua" );
	if ( true == luaL_dofile(lua, scriptPath.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL,
											 "File Not Found: "+scriptPath );
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
												   varName, scriptPath );
		}
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 varName, scriptPath );
		}
		lua_pop( lua, 1 );

		std::string tableName( "CountdownSprite" );
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
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				countdownSpritePath = lua_tostring(lua, TOP_IDX);
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
				const float temp = (float)lua_tointeger(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
															tableName+':'+field, scriptPath );
				}
				else
				{
					mSpriteClipSize.x = temp;
				}
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
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( 0 > temp )
				{
					gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
															tableName+':'+field, scriptPath );
				}
				else
				{
					mSpriteClipSize.y = temp;
				}
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
	lua_close( lua );

	if ( false == mTexture.loadFromFile(countdownSpritePath) )
	{
		gService()->console().printFailure( FailureLevel::WARNING,
												"File Not Found: "+countdownSpritePath );
	}

	mBackgroundRect_.setFillColor( sf::Color(backgroundColor) );
	mSprite.setTexture( mTexture );
	mSprite.setPosition( (sf::Vector2f(window.getSize())-mSpriteClipSize)*0.5f );
}

::scene::inPlay::ID scene::inPlay::Ready::update( std::vector<sf::Event>& )
{
	if ( 0 == mFrameCountToStart )
	{
		return ::scene::inPlay::ID::PLAYING;
	}

	return ::scene::inPlay::ID::AS_IS;
}

void ::scene::inPlay::Ready::draw( sf::RenderWindow& window )
{
	//
	// Background
	//
	window.draw( mBackgroundRect_ );

	//
	// Countdown
	//
	const sf::Vector2i cast( mSpriteClipSize );
	mSprite.setTextureRect( sf::IntRect( 0, cast.y*( mFrameCountToStart/mFPS_ ), cast.x, cast.y ) );
	window.draw( mSprite );

	--mFrameCountToStart;
}
