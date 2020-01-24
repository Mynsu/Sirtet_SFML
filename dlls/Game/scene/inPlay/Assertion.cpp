#include "../../pch.h"
#include "Assertion.h"
#include "../../ServiceLocatorMirror.h"
#include "../VaultKeyList.h"

scene::inPlay::Assertion::Assertion( sf::RenderWindow& window )
	: mFrameCount( 0 ), mFPS_( 60 ), mWindow_( window )
{
	auto& vault = gService()->vault();
	if ( const auto it = vault.find(HK_FORE_FPS);
		vault.cend() != it )
	{
		mFPS_ = (uint32_t)it->second;
	}
	mBackground.setSize( sf::Vector2f(window.getSize()) );
	mGuideTextLabel.setString( "Press ESC to quit." );
	loadResources( );
}

void scene::inPlay::Assertion::loadResources( )
{
	uint32_t backgroundColor = 0x0000007f;
	uint32_t fontSize = 50;
	uint32_t fontColor = 0x000000'af;
	std::string fontPathNName( "Fonts/AGENCYR.ttf" );

	lua_State* lua = luaL_newstate();
	const std::string scriptPathNName( "Scripts/Assertion.lua" );
	if ( true == luaL_dofile(lua, scriptPathNName.data()))
	{
		gService()->console().printFailure( FailureLevel::FATAL, "File Not Found: "+scriptPathNName );
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
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
												   varName, scriptPathNName );
		}
		lua_pop( lua, 1 );

		std::string tableName( "GuideTextLabel" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string field( "font" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				fontPathNName = lua_tostring(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService()->console().printScriptError(ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPathNName);
			}
			lua_pop( lua, 1 );

			field = "fontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				fontSize = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService()->console().printScriptError(ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPathNName);
			}
			lua_pop( lua, 1 );

			field = "fontColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				fontColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			else if ( LUA_TNIL == type )
			{
				gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
													   tableName+':'+field, scriptPathNName );
			}
			else
			{
				gService()->console().printScriptError(ExceptionType::TYPE_CHECK,
													   tableName+':'+field, scriptPathNName);
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	mBackground.setFillColor( sf::Color(backgroundColor) );
	if ( false == mFont.loadFromFile(fontPathNName) )
	{
		gService()->console().printFailure( FailureLevel::FATAL, "File Not Found: "+fontPathNName );
	}
	mGuideTextLabel.setCharacterSize( fontSize );
	mGuideTextLabel.setFillColor( sf::Color(fontColor) );
	mGuideTextLabel.setFont( mFont );
	const sf::FloatRect bound( mGuideTextLabel.getLocalBounds() );
	const sf::Vector2f size( bound.width, bound.height );
	mGuideTextLabel.setOrigin( size*0.5f );
	mGuideTextLabel.setPosition( sf::Vector2f(mWindow_.getSize())*0.5f );
}

::scene::inPlay::ID scene::inPlay::Assertion::update( std::list<sf::Event>& eventQueue )
{
	::scene::inPlay::ID retVal = ::scene::inPlay::ID::AS_IS;
	// 2 seconds after created,
	if ( 2*mFPS_ == mFrameCount )
	{
		retVal = ::scene::inPlay::ID::OFF;
	}
	else
	{
		for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; ++it )
		{
			if ( sf::Event::KeyPressed == it->type && sf::Keyboard::Escape == it->key.code )
			{
				it = eventQueue.erase( it );
				retVal = ::scene::inPlay::ID::MAIN_MENU;
			}
		}
	}

	return retVal;
}

void scene::inPlay::Assertion::draw( )
{
	mWindow_.draw( mBackground );
	mWindow_.draw( mGuideTextLabel );
	++mFrameCount;
}