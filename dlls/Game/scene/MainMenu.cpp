#include "../pch.h"
#include "MainMenu.h"
#include "../ServiceLocatorMirror.h"

bool ::scene::MainMenu::IsInstantiated = false;

::scene::MainMenu::MainMenu( const sf::RenderWindow& window )
	: mIsCursorOnButton( false ),
	mNextSceneID( ::scene::ID::AS_IS )
{
	ASSERT_TRUE( false == IsInstantiated );

	loadResources( window );

	IsInstantiated = true;
}

::scene::MainMenu::~MainMenu( )
{
	IsInstantiated = false;
}

void scene::MainMenu::loadResources( const sf::RenderWindow& )
{
	std::string spritePath( "Images/MainMenu.png" );
	mSoundPaths[(int)SoundIndex::BGM] = "Sounds/korobeiniki.mp3";
	mSoundPaths[(int)SoundIndex::SELECTION] = "Sounds/selection.wav";
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
	std::string fontPath( "Fonts/AGENCYB.ttf" );
	uint16_t fontSize = 30;
	sf::Vector2f copyrightPosition( 150.f, 150.f );
	std::string copyright;

	lua_State* lua = luaL_newstate();
	const std::string scriptPath( "Scripts/MainMenu.lua" );
	if ( true == luaL_dofile(lua, scriptPath.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL,
										   "File Not Found: "+scriptPath );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		std::string tableName( "Sprite" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPath );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				spritePath = lua_tostring(lua, TOP_IDX);
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

			std::string innerTableName( "logo" );
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPath );
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
																			tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.logoSourcePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.logoSourcePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.logoClipSize.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.logoClipSize.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.logoDestinationPosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.logoDestinationPosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
														 tableName+':'+innerTableName, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonSingleSourcePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonSingleSourcePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonSingleClipSize.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonSingleClipSize.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonSinglePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonSinglePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
														 tableName+':'+innerTableName, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonOnlineSourcePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonOnlineSourcePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonOnlineClipSize.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonOnlineClipSize.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonOnlinePosition.x = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
																 tableName+':'+field, scriptPath );
					}
					else
					{
						mDrawingInfo.buttonOnlinePosition.y = temp;
					}
				}
				else if ( LUA_TNIL == type )
				{
					gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
														   tableName+':'+innerTableName+':'+field, scriptPath );
				}
				else
				{
					gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
															 tableName+':'+innerTableName+':'+field, scriptPath );
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
													 tableName, scriptPath );
		}
		else
		{
			std::string field( "font" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				fontPath = lua_tostring(lua, TOP_IDX);
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
															 tableName+':'+field, scriptPath );
				}
				else
				{
					fontSize = temp;
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
															 tableName+':'+field, scriptPath );
				}
				else
				{
					copyrightPosition.x = temp;
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
															 tableName+':'+field, scriptPath );
				}
				else
				{
					copyrightPosition.y = temp;
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

		tableName = "Sound";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK, tableName, scriptPath );
		}
		else
		{
			std::string innerTableName( "BGM" );
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPath );
			}
			else
			{
				std::string field( "path" );
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				int type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					mSoundPaths[(int)SoundIndex::BGM] = lua_tostring(lua, TOP_IDX);
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

			innerTableName = "onSelection";
			lua_pushstring( lua, innerTableName.data() );
			lua_gettable( lua, 1 );
			if ( false == lua_istable(lua, TOP_IDX) )
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+innerTableName, scriptPath );
			}
			else
			{
				std::string field( "path" );
				lua_pushstring( lua, field.data() );
				lua_gettable( lua, 2 );
				int type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					mSoundPaths[(int)SoundIndex::SELECTION] = lua_tostring(lua, TOP_IDX);
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
		lua_pop( lua, 1 );
	}
	lua_close( lua );

	if ( false == mTexture.loadFromFile(spritePath) )
	{
		gService()->console().printFailure( FailureLevel::WARNING,
										   "File Not Found: "+spritePath );
	}
	mSprite.setTexture( mTexture );
	if ( false == mFont.loadFromFile(fontPath) )
	{
		gService()->console().printFailure( FailureLevel::WARNING,
										   "File Not Found: "+fontPath );
	}
	mTextLabelForCopyrightNotice.setCharacterSize( fontSize );
	mTextLabelForCopyrightNotice.setFont( mFont );
	mTextLabelForCopyrightNotice.setPosition( copyrightPosition );
	mTextLabelForCopyrightNotice.setString( copyright );
	if ( false == gService()->sound().playBGM(mSoundPaths[(int)SoundIndex::BGM], true) )
	{
		gService()->console().printFailure(FailureLevel::WARNING,
										   "File Not Found: "+mSoundPaths[(int)SoundIndex::BGM] );
	}
}

::scene::ID scene::MainMenu::update( std::vector<sf::Event>& eventQueue,
									const sf::RenderWindow& )
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

void ::scene::MainMenu::draw( sf::RenderWindow& window )
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
		const sf::Vector2f mousePos( sf::Mouse::getPosition()-window.getPosition() );
		const sf::FloatRect boundLogo( mDrawingInfo.logoDestinationPosition,
									  sf::Vector2f(mDrawingInfo.logoClipSize) );
		if ( true == boundLogo.contains(mousePos) )
		{
			// Logo
			const sf::Vector2i sourcePos( mDrawingInfo.logoSourcePosition.x + mDrawingInfo.logoClipSize.x,
										  mDrawingInfo.logoSourcePosition.y );
			mSprite.setTextureRect( sf::IntRect(sourcePos, mDrawingInfo.logoClipSize) );
			mSprite.setPosition( mDrawingInfo.logoDestinationPosition );
			window.draw( mSprite );
			// Copyright
			window.draw( mTextLabelForCopyrightNotice );
		}
		else if ( const sf::FloatRect boundButtonSingle(mDrawingInfo.buttonSinglePosition,
														sf::Vector2f(mDrawingInfo.buttonSingleClipSize));
				 true == boundButtonSingle.contains(mousePos) )
		{
			// Logo
			mSprite.setTextureRect( sf::IntRect(mDrawingInfo.logoSourcePosition,
												mDrawingInfo.logoClipSize) );
			mSprite.setPosition( mDrawingInfo.logoDestinationPosition );
			window.draw( mSprite );
			if ( true == sf::Mouse::isButtonPressed(sf::Mouse::Left) )
			{
				mNextSceneID = ::scene::ID::SINGLE_PLAY;
			}
			// Buttons
			const sf::Vector2i sourcePos( mDrawingInfo.buttonSingleSourcePosition.x + mDrawingInfo.buttonSingleClipSize.x,
										 mDrawingInfo.buttonSingleSourcePosition.y );
			mSprite.setTextureRect( sf::IntRect(sourcePos, mDrawingInfo.buttonSingleClipSize) );
			mSprite.setPosition( mDrawingInfo.buttonSinglePosition );
			window.draw( mSprite );
			mSprite.setTextureRect( sf::IntRect(mDrawingInfo.buttonOnlineSourcePosition,
												mDrawingInfo.buttonOnlineClipSize) );
			mSprite.setPosition( mDrawingInfo.buttonOnlinePosition );
			window.draw( mSprite );
			if ( false == mIsCursorOnButton )
			{
				mIsCursorOnButton = true;
				if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::SELECTION]) )
				{
					gService()->console().printFailure(FailureLevel::WARNING,
													   "File Not Found: "+mSoundPaths[(int)SoundIndex::SELECTION] );
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
			window.draw( mSprite );
			if ( true == sf::Mouse::isButtonPressed(sf::Mouse::Left) )
			{
				mNextSceneID = ::scene::ID::ONLINE_BATTLE;
			}
			// Buttons
			mSprite.setTextureRect( sf::IntRect(mDrawingInfo.buttonSingleSourcePosition,
												mDrawingInfo.buttonSingleClipSize) );
			mSprite.setPosition( mDrawingInfo.buttonSinglePosition );
			window.draw( mSprite );
			const sf::Vector2i sourcePos( mDrawingInfo.buttonOnlineSourcePosition.x + mDrawingInfo.buttonOnlineClipSize.x,
										 mDrawingInfo.buttonOnlineSourcePosition.y );
			mSprite.setTextureRect( sf::IntRect(sourcePos, mDrawingInfo.buttonOnlineClipSize) );
			mSprite.setPosition( mDrawingInfo.buttonOnlinePosition );
			window.draw( mSprite );
			if ( false == mIsCursorOnButton )
			{
				mIsCursorOnButton = true;
				if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::SELECTION]) )
				{
					gService()->console().printFailure(FailureLevel::WARNING,
													   "File Not Found: "+mSoundPaths[(int)SoundIndex::SELECTION] );
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
		window.draw( mSprite );
		mNextSceneID = ::scene::ID::AS_IS;
		mSprite.setPosition( mDrawingInfo.buttonSinglePosition );
		mSprite.setTextureRect( sf::IntRect(mDrawingInfo.buttonSingleSourcePosition,
											mDrawingInfo.buttonSingleClipSize) );
		window.draw( mSprite );
		mSprite.setPosition( mDrawingInfo.buttonOnlinePosition );
		mSprite.setTextureRect( sf::IntRect(mDrawingInfo.buttonOnlineSourcePosition,
											mDrawingInfo.buttonOnlineClipSize) );
		window.draw( mSprite );
		mIsCursorOnButton = false;
	}
}

::scene::ID scene::MainMenu::currentScene( ) const
{
	return ::scene::ID::MAIN_MENU;
}
