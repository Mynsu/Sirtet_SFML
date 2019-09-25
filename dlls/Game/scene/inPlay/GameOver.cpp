#include "../../pch.h"
#include "GameOver.h"
#include <Lib/ScriptLoader.h>
#include "../../ServiceLocatorMirror.h"

scene::inPlay::GameOver::GameOver( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: TARGET_ALPHA( 0x7fu ), mFade( 0xffu ), mFrameCount( 0u ),
	mWindow_( window ), mBackgroundRect_( static_cast<sf::RectangleShape&>(shapeOrSprite) )
{
	loadResources( );

	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS", 7 );
	mFPS_ = static_cast<uint32_t>((*glpService).vault( )[ HK_FORE_FPS ]);
}

void scene::inPlay::GameOver::loadResources( )
{
	sf::Vector2f size( 512.f, 256.f );
	bool isPathDefault = true;
	bool isWDefault = true;
	bool isHDefault = true;

	lua_State* lua = luaL_newstate( );
	const char scriptPathNName[] = "Scripts/GameOver.lua";
	if ( true == luaL_dofile(lua, scriptPathNName) )
	{
		// File Not Found Exception
		(*glpService).console()->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		const std::string tableName( "Sprite" );
		lua_getglobal( lua, tableName.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "path";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TSTRING == type )
			{
				if ( false == mTexture.loadFromFile(lua_tostring(lua, TOP_IDX)) )
				{
					// File Not Found Exception
					(*glpService).console( )->printScriptError( ExceptionType::FILE_NOT_FOUND,
																		(tableName+":"+field0).data(), scriptPathNName );
				}
				else
				{
					isPathDefault = false;
				}
			}
			// Type Check Exception
			else
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger( lua, TOP_IDX ));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName+":"+field1).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					size.x = temp;
					isWDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "height";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					(*glpService).console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName+":"+field2).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					size.y = temp;
					isHDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				(*glpService).console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName+":"+field2).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	if ( true == isPathDefault )
	{
		const char defaultFilePathNName[] = "Vfxs/Combo.png";
		if ( false == mTexture.loadFromFile(defaultFilePathNName) )
		{
			// Exception: When there's not even the default file,
			(*glpService).console( )->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}

	if ( true == isWDefault || true == isHDefault )
	{
		(*glpService).console( )->print( "Default: width 512, height 256" );
	}

	mSprite.setTexture( mTexture );
	mSprite.setTextureRect( sf::IntRect(sf::Vector2i(0, 0), sf::Vector2i(size)) );
	mSprite.setOrigin( size*0.5f );
	mSprite.setPosition( sf::Vector2f(mWindow_.getSize())*0.5f );
}

int8_t scene::inPlay::GameOver::update( ::scene::inPlay::IScene** const, std::list<sf::Event>& )
{
	int8_t retVal = 0;

	// When mFade reaches the target,
	if ( TARGET_ALPHA == mFade )
	{
		// Frame counting starts.
		++mFrameCount;
		// 3 seconds after,
		if ( 3*mFPS_ == mFrameCount )
		{
			retVal = 1;
		}
	}

	return retVal;
}

void scene::inPlay::GameOver::draw( )
{
	// Cyan
	if ( TARGET_ALPHA != mFade )
	{
		const uint32_t BACKGROUND_RGB = 0x29cdb500u;
		mFade -= 2u;
		mBackgroundRect_.setFillColor( sf::Color(BACKGROUND_RGB | mFade) );
	}
	mWindow_.draw( mBackgroundRect_ );
	if ( TARGET_ALPHA == mFade )
	{
		mWindow_.draw( mSprite );
	}
}
