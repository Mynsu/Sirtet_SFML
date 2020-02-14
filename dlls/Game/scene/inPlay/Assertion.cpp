#include "../../pch.h"
#include "Assertion.h"
#include <Lib/VaultKeyList.h>
#include "../../ServiceLocatorMirror.h"

scene::inPlay::Assertion::Assertion( sf::RenderWindow& window )
	: mFrameCountToCancel( 0 ), mFPS_( 60 ), mWindow_( window )
{
	auto& vault = gService()->vault();
	const auto it = vault.find(HK_FORE_FPS);
	ASSERT_TRUE( vault.end() != it );
	mFPS_ = (uint16_t)it->second;
	mBackground.setSize( sf::Vector2f(window.getSize()) );
	mTextLabelForGuide.setString( "Press ESC to quit." );
	loadResources( );
}

void scene::inPlay::Assertion::loadResources( )
{
	uint16_t fontSize = 50;
	uint32_t backgroundColor = 0x0000007f;
	uint32_t fontColor = 0x000000'af;
	std::string fontPathNName( "Fonts/AGENCYR.ttf" );
	mAudioList[(int)AudioIndex::ON_SELECTION] = "Audio/selection.wav";

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
				fontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
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

		tableName = "Audio";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string innerTableName( "onSelection" );
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
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
					mAudioList[(int)AudioIndex::ON_SELECTION] = lua_tostring(lua, TOP_IDX);
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );
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
	mTextLabelForGuide.setCharacterSize( fontSize );
	mTextLabelForGuide.setFillColor( sf::Color(fontColor) );
	mTextLabelForGuide.setFont( mFont );
	const sf::FloatRect bound( mTextLabelForGuide.getLocalBounds() );
	const sf::Vector2f size( bound.width, bound.height );
	mTextLabelForGuide.setOrigin( size*0.5f );
	mTextLabelForGuide.setPosition( sf::Vector2f(mWindow_.getSize())*0.5f );
}

::scene::inPlay::ID scene::inPlay::Assertion::update( std::vector<sf::Event>& eventQueue )
{
	::scene::inPlay::ID retVal = ::scene::inPlay::ID::AS_IS;
	// 2 seconds after created,
	if ( 2*mFPS_ == mFrameCountToCancel )
	{
		retVal = ::scene::inPlay::ID::UNDO;
	}
	else
	{
		for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
		{
			if ( sf::Event::KeyPressed == it->type &&
				sf::Keyboard::Escape == it->key.code )
			{
				if ( false == gService()->audio().playSFX(mAudioList[(int)AudioIndex::ON_SELECTION]) )
				{
					gService()->console().printFailure(FailureLevel::WARNING,
													   "File Not Found: "+mAudioList[(int)AudioIndex::ON_SELECTION] );
				}
				it = eventQueue.erase(it);
				retVal = ::scene::inPlay::ID::EXIT;
			}
			else
			{
				++it;
			}
		}
	}

	return retVal;
}

void scene::inPlay::Assertion::draw( )
{
	mWindow_.draw( mBackground );
	mWindow_.draw( mTextLabelForGuide );
	++mFrameCountToCancel;
}