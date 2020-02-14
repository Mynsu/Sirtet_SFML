#include "../pch.h"
#include "MainMenu.h"
#include <Lib/VaultKeyList.h>
#include "../ServiceLocatorMirror.h"

bool ::scene::MainMenu::IsInstantiated = false;

::scene::MainMenu::MainMenu( sf::RenderWindow& window )
	: mIsCursorOnButton( false ), mWindow_( window ),
	mNextSceneID( ::scene::ID::AS_IS )
{
	ASSERT_TRUE( false == IsInstantiated );

	loadResources( );

	IsInstantiated = true;
}

::scene::MainMenu::~MainMenu( )
{
	IsInstantiated = false;
}

void scene::MainMenu::loadResources( )
{
	std::string spritePathNName( "Images/MainMenu.png" );
	mAudioList[(int)AudioIndex::BGM] = "Audio/korobeiniki.mp3";
	mAudioList[(int)AudioIndex::ON_SELECTION] = "Audio/selection.wav";
	mDrawingInfo.logoSourcePosition.x = 0;
	mDrawingInfo.logoSourcePosition.y = 0;
	mDrawingInfo.logoClipSize.x = 256;
	mDrawingInfo.logoClipSize.y = 256;
	mDrawingInfo.logoDestinationPosition.x = 474.f;
	mDrawingInfo.logoDestinationPosition.y = 302.f;
	mDrawingInfo.buttonSingleSourcePosition.x = 0;
	mDrawingInfo.buttonSingleSourcePosition.y = 256;
	mDrawingInfo.buttonSingleClipSize.x = 256;
	mDrawingInfo.buttonSingleClipSize.y = 128;
	mDrawingInfo.buttonSinglePosition.x = 150.f;
	mDrawingInfo.buttonSinglePosition.y = 150.f;
	mDrawingInfo.buttonOnlineSourcePosition.x = 0;
	mDrawingInfo.buttonOnlineSourcePosition.y = 256+128;
	mDrawingInfo.buttonOnlineClipSize.x = 256;
	mDrawingInfo.buttonOnlineClipSize.y = 128;
	mDrawingInfo.buttonOnlinePosition.x = 150.f;
	mDrawingInfo.buttonOnlinePosition.y = 300.f;
	std::string fontPathNName( "Fonts/AGENCYB.ttf" );
	uint16_t fontSize = 30;
	sf::Vector2f copyrightPosition( 150.f, 150.f );
	std::string copyright;

	lua_State* lua = luaL_newstate();
	const std::string scriptPathNName( "Scripts/MainMenu.lua" );
	if ( true == luaL_dofile(lua, scriptPathNName.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL, "File Not Found: "+scriptPathNName );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		std::string tableName( "Sprite" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				spritePathNName = lua_tostring(lua, TOP_IDX);
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

			std::string innerTableName( "logo" );
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPathNName );
			}
			else
			{
				field = "sourceX";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																			tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.logoSourcePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "sourceY";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.logoSourcePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "clipWidth";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.logoClipSize.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "clipHeight";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.logoClipSize.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "destinationX";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const float temp = (float)lua_tonumber(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.logoDestinationPosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "destinationY";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const float temp = (float)lua_tonumber(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.logoDestinationPosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );
			}
			lua_pop( lua, 1 );

			innerTableName = "buttonSingle";
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPathNName );
			}
			else
			{
				field = "sourceX";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonSingleSourcePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "sourceY";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonSingleSourcePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "clipWidth";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonSingleClipSize.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "clipHeight";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonSingleClipSize.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "destinationX";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const float temp = (float)lua_tonumber(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonSinglePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "destinationY";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const float temp = (float)lua_tonumber(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonSinglePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );
			}
			lua_pop( lua, 1 );

			innerTableName = "buttonOnline";
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPathNName );
			}
			else
			{
				field = "sourceX";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonOnlineSourcePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "sourceY";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonOnlineSourcePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "clipWidth";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonOnlineClipSize.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "clipHeight";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const int32_t temp = (int32_t)lua_tointeger(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonOnlineClipSize.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "destinationX";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const float temp = (float)lua_tonumber(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonOnlinePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );

				field = "destinationY";
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TNUMBER == type )
				{
					const float temp = (float)lua_tonumber(lua, TOP_IDX);
					if ( 0 > temp )
					{
						gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
																 tableName+':'+field, scriptPathNName );
					}
					else
					{
						mDrawingInfo.buttonOnlinePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPathNName );
				}
				lua_pop( lua, 1 );
			}
			lua_pop( lua, 1 );
		}
		lua_pop( lua, 1 );

		tableName = "Copyright";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "font" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
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
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "fontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const uint16_t temp = (uint16_t)lua_tointeger(lua, TOP_IDX);
				if ( temp < 0 )
				{
					gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
															 tableName+':'+field, scriptPathNName );
				}
				else
				{
					fontSize = temp;
				}
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

			field = "positionX";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( temp < 0 )
				{
					gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
															 tableName+':'+field, scriptPathNName );
				}
				else
				{
					copyrightPosition.x = temp;
				}
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

			field = "positionY";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				const float temp = (float)lua_tonumber(lua, TOP_IDX);
				if ( temp < 0 )
				{
					gService()->console().printScriptError( ExceptionType::RANGE_CHECK,
															 tableName+':'+field, scriptPathNName );
				}
				else
				{
					copyrightPosition.y = temp;
				}
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

			field = "text";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				copyright = lua_tostring(lua, TOP_IDX);
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

		tableName = "Audio";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPathNName );
		}
		else
		{
			std::string innerTableName( "BGM" );
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
					mAudioList[(int)AudioIndex::BGM] = lua_tostring(lua, TOP_IDX);
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

			innerTableName = "onSelection";
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

	if ( false == mTexture.loadFromFile(spritePathNName) )
	{
		// Exception: When there's not even the default file,
		gService()->console().printFailure( FailureLevel::FATAL, "File Not Found: "+spritePathNName );
	}
	mSprite.setTexture( mTexture );
	if ( false == mFont.loadFromFile(fontPathNName) )
	{
		gService()->console().printFailure( FailureLevel::FATAL, "File Not Found: "+fontPathNName );
	}
	mCopyright.setCharacterSize( fontSize );
	mCopyright.setFont( mFont );
	mCopyright.setPosition( copyrightPosition );
	mCopyright.setString( copyright );
	if ( false == gService()->audio().playBGM(mAudioList[(int)AudioIndex::BGM], true) )
	{
		gService()->console().printFailure(FailureLevel::WARNING,
										   "File Not Found: "+mAudioList[(int)AudioIndex::BGM] );
	}
}

::scene::ID scene::MainMenu::update( std::vector<sf::Event>& eventQueue )
{
	for ( const sf::Event& it : eventQueue )
	{
		if ( sf::Event::KeyPressed == it.type &&
			sf::Keyboard::Escape == it.key.code )
		{
			auto& vault = gService()->vault();
			const auto it = vault.find(HK_IS_RUNNING);
			ASSERT_TRUE( vault.end() != it );
			it->second = 0;
		}
	}

	return mNextSceneID;
}

void ::scene::MainMenu::draw( )
{
	bool hasGainedFocus = false;
	auto& vault = gService()->vault();
	{
		const auto it = vault.find(HK_HAS_GAINED_FOCUS);
		ASSERT_TRUE( vault.end() != it );
		hasGainedFocus = (bool)it->second;
	}

	if ( true == hasGainedFocus && false == gService()->console().isVisible() )
	{
		const sf::Vector2f mousePos( sf::Mouse::getPosition()-mWindow_.getPosition() );
		const sf::FloatRect boundLogo( mDrawingInfo.logoDestinationPosition,
									  sf::Vector2f(mDrawingInfo.logoClipSize) );
		if ( true == boundLogo.contains(mousePos) )
		{
			// Logo
			const sf::Vector2i sourcePos( mDrawingInfo.logoSourcePosition.x + mDrawingInfo.logoClipSize.x,
										  mDrawingInfo.logoSourcePosition.y );
			mSprite.setTextureRect( sf::IntRect(sourcePos, mDrawingInfo.logoClipSize) );
			mSprite.setPosition( mDrawingInfo.logoDestinationPosition );
			mWindow_.draw( mSprite );
			// Copyright
			mWindow_.draw( mCopyright );
		}
		else if ( const sf::FloatRect boundButtonSingle(mDrawingInfo.buttonSinglePosition,
														sf::Vector2f(mDrawingInfo.buttonSingleClipSize));
				 true == boundButtonSingle.contains(mousePos) )
		{
			// Logo
			mSprite.setTextureRect( sf::IntRect(mDrawingInfo.logoSourcePosition,
												mDrawingInfo.logoClipSize) );
			mSprite.setPosition( mDrawingInfo.logoDestinationPosition );
			mWindow_.draw( mSprite );
			if ( true == sf::Mouse::isButtonPressed(sf::Mouse::Left) )
			{
				mNextSceneID = ::scene::ID::SINGLE_PLAY;
			}
			// Buttons
			const sf::Vector2i sourcePos( mDrawingInfo.buttonSingleSourcePosition.x + mDrawingInfo.buttonSingleClipSize.x,
										 mDrawingInfo.buttonSingleSourcePosition.y );
			mSprite.setTextureRect( sf::IntRect(sourcePos, mDrawingInfo.buttonSingleClipSize) );
			mSprite.setPosition( mDrawingInfo.buttonSinglePosition );
			mWindow_.draw( mSprite );
			mSprite.setTextureRect( sf::IntRect(mDrawingInfo.buttonOnlineSourcePosition,
												mDrawingInfo.buttonOnlineClipSize) );
			mSprite.setPosition( mDrawingInfo.buttonOnlinePosition );
			mWindow_.draw( mSprite );
			if ( false == mIsCursorOnButton )
			{
				mIsCursorOnButton = true;
				if ( false == gService()->audio().playSFX(mAudioList[(int)AudioIndex::ON_SELECTION]) )
				{
					gService()->console().printFailure(FailureLevel::WARNING,
													   "File Not Found: "+mAudioList[(int)AudioIndex::ON_SELECTION] );
				}
			}
		}
		else if ( const sf::FloatRect boundButtonOnline(mDrawingInfo.buttonOnlinePosition,
														sf::Vector2f(mDrawingInfo.buttonOnlineClipSize));
				 true == boundButtonOnline.contains(mousePos) )
		{
			// Logo
			mSprite.setTextureRect( sf::IntRect(mDrawingInfo.logoSourcePosition,
												mDrawingInfo.logoClipSize) );
			mSprite.setPosition( mDrawingInfo.logoDestinationPosition );
			mWindow_.draw( mSprite );
			if ( true == sf::Mouse::isButtonPressed(sf::Mouse::Left) )
			{
				mNextSceneID = ::scene::ID::ONLINE_BATTLE;
			}
			// Buttons
			mSprite.setTextureRect( sf::IntRect(mDrawingInfo.buttonSingleSourcePosition,
												mDrawingInfo.buttonSingleClipSize) );
			mSprite.setPosition( mDrawingInfo.buttonSinglePosition );
			mWindow_.draw( mSprite );
			const sf::Vector2i sourcePos( mDrawingInfo.buttonOnlineSourcePosition.x + mDrawingInfo.buttonOnlineClipSize.x,
										 mDrawingInfo.buttonOnlineSourcePosition.y );
			mSprite.setTextureRect( sf::IntRect(sourcePos, mDrawingInfo.buttonOnlineClipSize) );
			mSprite.setPosition( mDrawingInfo.buttonOnlinePosition );
			mWindow_.draw( mSprite );
			if ( false == mIsCursorOnButton )
			{
				mIsCursorOnButton = true;
				if ( false == gService()->audio().playSFX(mAudioList[(int)AudioIndex::ON_SELECTION]) )
				{
					gService()->console().printFailure(FailureLevel::WARNING,
													   "File Not Found: "+mAudioList[(int)AudioIndex::ON_SELECTION] );
				}
			}
		}
		else
		{
			goto defaultLabel;
		}
	}
	else
	{
defaultLabel:
		mSprite.setTextureRect( sf::IntRect(mDrawingInfo.logoSourcePosition,
											mDrawingInfo.logoClipSize) );
		mSprite.setPosition( mDrawingInfo.logoDestinationPosition );
		mWindow_.draw( mSprite );
		mNextSceneID = ::scene::ID::AS_IS;
		mSprite.setPosition( mDrawingInfo.buttonSinglePosition );
		mSprite.setTextureRect( sf::IntRect(mDrawingInfo.buttonSingleSourcePosition,
											mDrawingInfo.buttonSingleClipSize) );
		mWindow_.draw( mSprite );
		mSprite.setPosition( mDrawingInfo.buttonOnlinePosition );
		mSprite.setTextureRect( sf::IntRect(mDrawingInfo.buttonOnlineSourcePosition,
											mDrawingInfo.buttonOnlineClipSize) );
		mWindow_.draw( mSprite );
		mIsCursorOnButton = false;
	}
}

#ifdef _DEV
::scene::ID scene::MainMenu::currentScene( ) const
{
	return ::scene::ID::MAIN_MENU;
}
#endif