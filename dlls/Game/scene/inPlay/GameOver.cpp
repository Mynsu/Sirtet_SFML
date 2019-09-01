#include "GameOver.h"
#include <Lib/ScriptLoader.h>
#include "../../ServiceLocatorMirror.h"

scene::inPlay::GameOver::GameOver( sf::RenderWindow& window, sf::Drawable& shapeOrSprite )
	: mFade( 0xffu ),
	mWindow_( window ), mBackgroundRect_( static_cast<sf::RectangleShape&>(shapeOrSprite) )
{
	loadResources( );
}

void scene::inPlay::GameOver::loadResources( )
{
	std::string filePathNName( "Images/GameOver.png" );
	sf::Vector2i size( 512, 256 );
	bool isDefault = true;

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/GameOver.lua" );
	if ( true == luaL_dofile( lua, scriptPathNName.data( ) ) )
	{
		// File Not Found Exception
		ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, "File Not Found: " + scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		const std::string tableName0( "Sprite" );
		lua_getglobal( lua, tableName0.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0, scriptPathNName );
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
				if ( false == mTexture.loadFromFile( lua_tostring( lua, TOP_IDX ) ) )
				{
					// File Not Found Exception
					ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::FILE_NOT_FOUND,
																		tableName0+":"+field0, scriptPathNName );
				}
				else
				{
					isDefault = false;
				}
			}
			// Type Check Exception
			else
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0+":"+field0, scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				size.x = static_cast<int>(lua_tointeger( lua, TOP_IDX ));
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0+":"+field1, scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "height";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				size.y = static_cast<int>(lua_tointeger( lua, TOP_IDX ));
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				ServiceLocatorMirror::Console( )->printScriptError( ExceptionType::TYPE_CHECK, tableName0+":"+field1, scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	if ( true == isDefault )
	{
		const std::string defaultFilePathNName( "Vfxs/Combo.png" );
		if ( false == mTexture.loadFromFile( defaultFilePathNName ) )
		{
			// Exception: When there's not even the default file,
			ServiceLocatorMirror::Console( )->printFailure( FailureLevel::FATAL, "File Not Found: " + defaultFilePathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}

	mSprite.setTexture( mTexture );
	mSprite.setTextureRect( sf::IntRect(0, 0, size.x, size.y) );
	mSprite.setOrigin( sf::Vector2f(size)*0.5f );
	mSprite.setPosition( sf::Vector2f( mWindow_.getSize( ) )*0.5f );
}

void scene::inPlay::GameOver::update( ::scene::inPlay::IScene** const nextScene, std::queue<sf::Event>& eventQueue )
{
}

void scene::inPlay::GameOver::draw( )
{
	// Cyan
	const uint32_t BACKGROUND_RGB = 0x29cdb500u;
	if ( 0x7fu != mFade )
	{
		--mFade;
	}
	mBackgroundRect_.setFillColor( sf::Color( BACKGROUND_RGB | mFade ) );
	mWindow_.draw( mBackgroundRect_ );

	if ( 0x7fu == mFade )
	{
		mWindow_.draw( mSprite );
	}
}
