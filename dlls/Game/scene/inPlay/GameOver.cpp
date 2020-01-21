#include "../../pch.h"
#include "GameOver.h"
#include <Lib/ScriptLoader.h>
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"

scene::inPlay::GameOver::GameOver( sf::RenderWindow& window, sf::Drawable& shapeOrSprite,
								   std::unique_ptr<::scene::inPlay::IScene>& overlappedScene )
	: TARGET_ALPHA( 0x7fu ), mFade( 0xffu ), mFrameCount( 0u ),
	mWindow_( window ), mBackgroundRect_( (sf::RectangleShape&)shapeOrSprite )
{
	overlappedScene.reset( );

	loadResources( );

	mFPS_ = (uint32_t)gService( )->vault( )[ HK_FORE_FPS ];
}

void scene::inPlay::GameOver::loadResources( )
{
	sf::Vector2f imageSize( 512.f, 256.f );
	std::string imagePathNName( "Images/GameOver.png" );

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/GameOver.lua" );
	if ( true == luaL_dofile(lua, scriptPathNName.data()) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL, "File Not Found: "+scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		std::string tableName( "Image" );
		lua_getglobal( lua, tableName.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type Check Exception
			if ( LUA_TSTRING != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			else
			{
				imagePathNName = lua_tostring(lua, TOP_IDX);
			}
			lua_pop( lua, 1 );

			field = "width";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				// Range Check Exception
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
																		tableName+':'+field, scriptPathNName );
				}
				else
				{
					imageSize.x = temp;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																	tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "height";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				// Range Check Exception
				if ( 0 > temp )
				{
					gService( )->console( ).printScriptError( ExceptionType::RANGE_CHECK,
																		tableName+':'+field, scriptPathNName );
				}
				else
				{
					imageSize.y = temp;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																	tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );
		lua_close( lua );
	}

	if ( false == mTexture.loadFromFile(imagePathNName) )
	{
		gService( )->console( ).printFailure( FailureLevel::FATAL, "File Not Found: "+imagePathNName );
#ifdef _DEBUG
		__debugbreak( );
#endif
	}

	mSprite.setTexture( mTexture );
	mSprite.setTextureRect( sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(imageSize)) );
	mSprite.setOrigin( imageSize*0.5f );
	mSprite.setPosition( sf::Vector2f(mWindow_.getSize())*0.5f );
}

::scene::inPlay::ID scene::inPlay::GameOver::update( std::list<sf::Event>& )
{
	::scene::inPlay::ID retVal = ::scene::inPlay::ID::AS_IS;

	// When mFade reaches the target,
	if ( mFade <= TARGET_ALPHA )
	{
		// Frame counting starts.
		++mFrameCount;
		// 3 seconds after,
		if ( 3*mFPS_ == mFrameCount )
		{
			retVal = ::scene::inPlay::ID::MAIN_MENU;
		}
	}

	return retVal;
}

void scene::inPlay::GameOver::draw( )
{
	// Cyan
	if ( TARGET_ALPHA < mFade )
	{
		const uint32_t BACKGROUND_RGB = 0x29cdb500u;
		mFade -= 2u;
		mBackgroundRect_.setFillColor( sf::Color(BACKGROUND_RGB | mFade) );
		mWindow_.draw( mBackgroundRect_ );
	}
	else
	{
		mWindow_.draw( mBackgroundRect_ );
		mWindow_.draw( mSprite );
	}
}
