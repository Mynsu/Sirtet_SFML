#include "../../pch.h"
#include "GameOver.h"
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"

scene::inPlay::GameOver::GameOver( sf::RenderWindow& window, sf::Drawable& shapeOrSprite,
								   std::unique_ptr<::scene::inPlay::IScene>& overlappedScene )
	: TARGET_ALPHA( 0x7fu ), mFade( 0xffu ), mFrameCount( 0 ),
	mWindow_( window ), mBackgroundRect_( (sf::RectangleShape&)shapeOrSprite )
{
	overlappedScene.reset( );
	auto& vault = gService()->vault();
	if ( auto it = vault.find(HK_FORE_FPS);
		vault.end() != it )
	{
		mFPS_ = (uint32_t)it->second;
	}
#ifdef _DEBUG
	else
	{
		__debugbreak( );
	}
#endif
	loadResources( );
	if ( false == gService()->audio().playBGM(mAudioList[(int)AudioIndex::BGM]) )
	{
		gService()->console().printFailure(FailureLevel::WARNING,
										   "File Not Found: "+mAudioList[(int)AudioIndex::BGM] );
	}
}

void scene::inPlay::GameOver::loadResources( )
{
	sf::Vector2f imageSize( 512.f, 256.f );
	std::string imagePathNName( "Images/GameOver.png" );
	mAudioList[(int)AudioIndex::BGM] = "Audio/gameOver.wav";

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/GameOver.lua" );
	if ( true == luaL_dofile(lua, scriptPathNName.data()) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL, "File Not Found: "+scriptPathNName );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;
		std::string tableName( "Image" );
		lua_getglobal( lua, tableName.data() );
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
			if ( LUA_TSTRING == type )
			{
				imagePathNName = lua_tostring(lua, TOP_IDX);
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

			field = "width";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
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
					imageSize.x = temp;
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

			field = "height";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
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
					imageSize.y = temp;
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

		tableName = "Audio";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string innerTableName( "BGM" );
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPathNName );
			}
			else
			{
				std::string field( "path" );
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				int type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					mAudioList[(int)AudioIndex::BGM] = lua_tostring(lua, TOP_IDX);
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
		lua_pop( lua, 1 );
	}
	lua_close( lua );

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

::scene::inPlay::ID scene::inPlay::GameOver::update( std::vector<sf::Event>& )
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
			retVal = ::scene::inPlay::ID::EXIT;
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
