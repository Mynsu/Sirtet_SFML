#include "Ready.h"
#include "../../ServiceLocatorMirror.h"
#include <Lib/ScriptLoader.h>
#include "Playing.h"

scene::inPlay::Ready::Ready( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mFPS_( 60u ), mFrameCount( mFPS_ * 3 ),
	mWindow_( window ), mBackgroundRect_( static_cast< sf::RectangleShape& >( shapeOrSprite ) ),
	mSpriteClipSize_( 256.f, 256.f )
{
	auto& varT = ::ServiceLocatorMirror::Vault( );
	constexpr HashedKey HK_FORE_FPS = ::util::hash::Digest( "foreFPS" );
	if ( const auto it = varT.find( HK_FORE_FPS ); varT.cend( ) != it )
	{
		mFPS_ = it->second;
		mFrameCount = mFPS_ * 3;
	}

	loadResources( );

	// Cyan
	const uint32_t BACKGROUND_RGB = 0x29cdb500u;
	const uint32_t FADE = 0x7fu;
	mBackgroundRect_.setFillColor( sf::Color( BACKGROUND_RGB | FADE ) );
}

void scene::inPlay::Ready::loadResources( )
{
	bool isPathDefault = true;
	bool isWDefault = true;
	bool isHDefault = true;

	lua_State* lua = luaL_newstate( );
	const char scriptPathNName[ ] = "Scripts/Ready.lua";
	if ( true == luaL_dofile(lua, scriptPathNName) )
	{
		// File Not Found Exception
		ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		const std::string tableName0( "Sprite" );
		lua_getglobal( lua, tableName0.data( ) );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0.data( ), scriptPathNName );
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
					ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::FILE_NOT_FOUND,
																		(tableName0+":"+field0).data( ), scriptPathNName );
				}
				else
				{
					isPathDefault = false;
				}
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName0+":"+field0).data( ), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				const float temp = static_cast<float>(lua_tointeger(lua, TOP_IDX));
				// Range Check Exception
				if ( 0 > temp )
				{
					ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName0+":"+field1).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mSpriteClipSize_.x = temp;
					isWDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName0+":"+field1).data( ), scriptPathNName );
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
					ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::RANGE_CHECK,
																		(tableName0+":"+field2).data( ), scriptPathNName );
				}
				// When the value looks OK,
				else
				{
					mSpriteClipSize_.y = temp;
					isHDefault = false;
				}
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK,
																	(tableName0+":"+field2).data( ), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	if ( true == isPathDefault )
	{
		const char defaultFilePathNName[ ] = "Images/Ready.png";
		if ( false == mTexture.loadFromFile(defaultFilePathNName) )
		{
			// Exception: When there's not even the default file,
			ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}

	if ( true == isWDefault || true == isHDefault )
	{
		ServiceLocatorMirror::Console( )->print( "Default: width 256, height 256" );
	}

	mSprite.setTexture( mTexture );
	mSprite.setPosition( (sf::Vector2f(mWindow_.getSize())-mSpriteClipSize_)*0.5f );
}

void scene::inPlay::Ready::update( ::scene::inPlay::IScene** const nextScene, std::queue< sf::Event >& eventQueue )
{
	// NOTE: moved into 'draw( ).'
	///--mFrameCount;

	if ( 0 == mFrameCount )
	{
		*nextScene = new ::scene::inPlay::Playing( mWindow_, mBackgroundRect_ );
	}
}

void scene::inPlay::Ready::draw( )
{
	////
	// Background
	////

	// Cyan
	const uint32_t BACKGROUND_RGB = 0x29cdb500u;
	const uint32_t FADE = 0x7fu;
	mBackgroundRect_.setFillColor( sf::Color( BACKGROUND_RGB | FADE ) );
	mWindow_.draw( mBackgroundRect_ );

	////
	// Countdown
	////
	const sf::Vector2i cast( mSpriteClipSize_ );
	mSprite.setTextureRect( sf::IntRect( 0, cast.y*( mFrameCount/mFPS_ ), cast.x, cast.y ) );
	mWindow_.draw( mSprite );

	--mFrameCount;
}
