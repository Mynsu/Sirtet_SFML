#include "../../pch.h"
#include "InLobby.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "Online.h"
#include "../VaultKeyList.h"

const uint32_t UPDATE_USER_LIST_INTERVAL = 300;
const uint32_t REQUEST_DELAY = 60;
const float ERROR_RANGE = 5.f;
const float SPEED = .5f;
std::mutex MutexForGuideText;

bool scene::online::InLobby::IsInstantiated = false;

scene::online::InLobby::InLobby( sf::RenderWindow& window, ::scene::online::Online& net )
	: mIsReceiving( false ), mHasJoined( false ),
	mGuideTextIndex( 0 ),
	mFrameCount_update( 30 ), mFrameCount_requestDelay( 0 ), enteringRoom( 0 ),
	mWindow_( window ), mNet( net ),
	mTextInputBox( window )
{
	ASSERT_TRUE( false == IsInstantiated );

	const sf::Vector2f winSize( mWindow_.getSize() );
	mBackground.setSize( winSize );
	const std::string& myNickname = mNet.myNickname();
	sf::Text tf( myNickname, mFont_ );
	tf.setOrigin( 0.5f, 0.5f );
	tf.setPosition(	winSize*.5f );
	tf.setFillColor( sf::Color(0xffa500ff) ); // Orange
	mUserList.emplace( myNickname, std::make_pair(tf,0) );
	loadResources( );
#ifdef _DEV
	IServiceLocator* const service = gService();
	ASSERT_NOT_NULL( service );
	service->console( ).addCommand( CMD_CREATE_ROOM, std::bind(&scene::online::InLobby::_createRoom,
																	  this, std::placeholders::_1) );
	service->console( ).addCommand( CMD_JOIN_ROOM, std::bind(&scene::online::InLobby::joinRoom,
																		 this, std::placeholders::_1) );
#endif

	IsInstantiated = true;
}

scene::online::InLobby::~InLobby( )
{
#ifdef _DEV
	gService( )->console( ).removeCommand( CMD_CREATE_ROOM );
	gService( )->console( ).removeCommand( CMD_JOIN_ROOM );
#endif
	IsInstantiated = false;
}

void scene::online::InLobby::loadResources( )
{
	uint32_t backgroundColor = 0x29cdb5fa; // Cyan
	sf::Vector2f boxPosition( 0.f, 100.f );
	sf::Vector2f boxSize( 800.f, 400.f );
	uint32_t boxColor = 0x000000ff;
	float boxOutlineThickness = 5.f;
	uint32_t boxOutlineColor = 0x0000007f;
	mMovingPoints = { {0.f, 490.f}, {0.f, 110.f}, {600.f, 110.f}, {600.f, 490.f} };
	sf::Vector2f guideTextLabelPosition( 400.f, 200.f );
	uint32_t guideTextColor = 0xffffff7f;
	mGuideTexts.clear( );
	mGuideTexts.emplace_back( "Double click to create a room." );
	mGuideTexts.emplace_back( "Right click to join a room." );
	uint32_t guideTextFontSize = 20;
	std::string guideTextFont( "Fonts/AGENCYB.TTF" );
	uint32_t shade = 0x0000007f;
	sf::Vector2f subWindowPosition( 300.f, 200.f );
	sf::Vector2f subWindowSize( 200.f, 100.f );
	uint32_t subWindowColor = 0xffff00ff;
	std::string subWindowTitleFont( "Fonts/AGENCYB.TTF" );
	sf::Vector2f subWindowTitleRelativePosition( 20.f, 10.f );
	uint32_t subWindowTitleFontSize = 30;
	uint32_t subWindowTitleFontColor = 0xffffffff;
	sf::Vector2f subWindowInputTextFieldRelativePosition( 20.f, 60.f );
	uint32_t subWindowInputTextFieldFontSize = 25;
	uint32_t subWindowInputTextFieldFontColor = 0xffffffff;

	lua_State* lua = luaL_newstate( );
	std::string scriptPathNName( "Scripts/InLobby.lua" );
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

		std::string varName( "BackgroundColor" );
		lua_getglobal( lua, varName.data() );
		int type = lua_type(lua, TOP_IDX);
		if ( LUA_TNUMBER == type )
		{
			backgroundColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
		}
		else
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 varName, scriptPathNName );
		}
		lua_pop( lua, 1 );

		std::string tableName( "Box" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			// Type Check Exception
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "x" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				boxPosition.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				boxPosition.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "width";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				boxSize.x = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				boxSize.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				boxColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "outlineThickness";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				boxOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "outlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				boxOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

		tableName = "MovingPoints";
		lua_getglobal( lua, tableName.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			mMovingPoints.clear();
			uint8_t i = 1;
			while ( true )
			{
				lua_pushinteger( lua, i );
				lua_gettable( lua, 1 );
				if ( true == lua_istable(lua, TOP_IDX) )
				{
					uint8_t k = 1;
					sf::Vector2f point;
					lua_pushinteger( lua, k );
					lua_gettable( lua, 2 );
					type = lua_type(lua, TOP_IDX);
					if ( LUA_TNUMBER == type )
					{
						point.x = (float)lua_tonumber(lua, TOP_IDX);
					}
					// Type Check Exception
					else if ( LUA_TNIL != type )
					{
						gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																"", scriptPathNName );
					}
					lua_pop( lua, 1 );

					k = 2;
					lua_pushinteger( lua, k );
					lua_gettable( lua, 2 );
					type = lua_type(lua, TOP_IDX);
					if ( LUA_TNUMBER == type )
					{
						point.y = (float)lua_tonumber(lua, TOP_IDX);
					}
					// Type Check Exception
					else if ( LUA_TNIL != type )
					{
						gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
																"", scriptPathNName );
					}
					mMovingPoints.emplace_back( point );
					++i;
					lua_pop( lua, 2 );
				}
				else
				{
					lua_pop( lua, 1 );
					break;
				}
			}
		}
		lua_pop( lua, 1 );

		tableName = "GuideTextLabel";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			// Type Check Exception
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "x" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				guideTextLabelPosition.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				guideTextLabelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				guideTextColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "font";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				guideTextFont = lua_tostring(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "fontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				guideTextFontSize = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

		tableName = "GuideTexts";
		lua_getglobal( lua, tableName.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::scoped_lock lock( MutexForGuideText );
			mGuideTexts.clear();
			uint8_t i = 1;
			while ( true )
			{
				lua_pushinteger( lua, i );
				lua_gettable( lua, 1 );
				type = lua_type(lua, TOP_IDX);
				if ( LUA_TSTRING == type )
				{
					mGuideTexts.emplace_back( std::string(lua_tostring(lua, TOP_IDX)) );
					++i;
					lua_pop( lua, 1 );
				}
				else
				{
					lua_pop( lua, 1 );
					break;
				}
			}
		}
		lua_pop( lua, 1 );

		tableName = "SubWindow";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			// Type Check Exception
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "shade" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				shade = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "x";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowPosition.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowPosition.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "width";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowSize.x = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowSize.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "font";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				subWindowTitleFont = lua_tostring(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "titleRelativeX";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowTitleRelativePosition.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "titleRelativeY";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowTitleRelativePosition.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "titleFontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowTitleFontSize = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "titleFontColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowTitleFontColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "inputTextFieldX";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowInputTextFieldRelativePosition.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "inputTextFieldY";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowInputTextFieldRelativePosition.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "inputTextFieldFontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowInputTextFieldFontSize = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "inputTextFieldFontColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowInputTextFieldFontColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
	}
	lua_close( lua );

	mBackground.setFillColor( sf::Color(backgroundColor) );
	mUserNicknamesBox.setPosition( boxPosition );
	mUserNicknamesBox.setSize( boxSize );
	mUserNicknamesBox.setFillColor( sf::Color(boxColor) );
	mUserNicknamesBox.setOutlineThickness( boxOutlineThickness );
	mUserNicknamesBox.setOutlineColor( sf::Color(boxOutlineColor) );
	mFont_.loadFromFile( guideTextFont );
	mGuideTextLabel.setPosition( guideTextLabelPosition );
	mGuideTextLabel.setFillColor( sf::Color(guideTextColor) );
	mGuideTextLabel.setCharacterSize( guideTextFontSize );
	mGuideTextLabel.setFont( mFont_ );
	mTextInputBox.setBackgroundColor( sf::Color(shade) );
	mTextInputBox.setPosition( subWindowPosition );
	mTextInputBox.setSize( subWindowSize );
	mTextInputBox.setColor( sf::Color(subWindowColor) );
	mTextInputBox.loadFont( subWindowTitleFont );
	mTextInputBox.setTitleDimension( subWindowTitleRelativePosition, subWindowTitleFontSize );
	mTextInputBox.setTitleColor( sf::Color(subWindowTitleFontColor) );
	mTextInputBox.setInputTextFieldDimension( subWindowInputTextFieldRelativePosition,
											 subWindowInputTextFieldFontSize );
	mTextInputBox.setInputTextFieldColor( sf::Color(subWindowInputTextFieldFontColor) );

	mDestination_.mComponents[0] = 100.f;
	mDestination_.mComponents[1] = 0.f;
	scriptPathNName = "Scripts/InRoom.lua";
	lua = luaL_newstate( );
	if ( true == luaL_dofile(lua, scriptPathNName.data()) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL, "File Not Found: "+scriptPathNName );
	}
	else
	{
		const int TOP_IDX = -1;

		std::string tableName( "PlayerPanel" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			// Type Check Exception
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "x" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDestination_.mComponents[0] = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPathNName );
			}
			lua_pop( lua, 1 );

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDestination_.mComponents[1] = (float)lua_tonumber(lua, TOP_IDX);
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
	}
	lua_pop( lua, 1 );
	lua_close( lua );
	math::Vector<2> dir( mDestination_.mComponents[0]-boxPosition.x,
						mDestination_.mComponents[1]-boxPosition.y );
	mAcceleration_ = dir.normalize()*SPEED;
}

::scene::online::ID scene::online::InLobby::update( std::list<sf::Event>& eventQueue )
{
	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	if ( 2 == enteringRoom )
	{
		retVal = ::scene::online::ID::IN_ROOM;
		return retVal;
	}
	else if ( -2 == enteringRoom )
	{
		retVal = ::scene::online::ID::IN_ROOM_AS_HOST;
		return retVal;
	}

	if ( true == mNet.hasReceived() )
	{
		mIsReceiving = false;
		if ( std::optional<std::string> resultCreatingRoom( mNet.getByTag(TAGGED_REQ_CREATE_ROOM,
															   Online::Option::RETURN_TAG_ATTACHED,
															   -1) );
			std::nullopt != resultCreatingRoom )
		{
			enteringRoom = -1;
		}
		else if ( std::optional<std::string> resultJoiningRoom( mNet.getByTag(TAGGED_REQ_JOIN_ROOM,
																			 Online::Option::DEFAULT,
																			 sizeof(uint8_t)) );
				 std::nullopt != resultJoiningRoom )
		{
			const ResultJoiningRoom res = (ResultJoiningRoom)*resultJoiningRoom.value().data();
			switch( res )
			{
				case ResultJoiningRoom::FAILED_BY_SERVER_ERROR:
					gService()->console().print( "Failed to join the room by an error.", sf::Color::Green );
					break;
				case ResultJoiningRoom::FAILED_BY_FULL_ROOM:
					gService()->console().print( "Room is full.", sf::Color::Green );
					break;
				case ResultJoiningRoom::SUCCCEDED:
					enteringRoom = 1;
					break;
				case ResultJoiningRoom::FAILED_DUE_TO_TARGET_NOT_CONNECTING:
					gService()->console().print( "That nicknamed-user is not connecting.", sf::Color::Green );
					break;
				case ResultJoiningRoom::FAILED_DUE_TO_SELF_TARGET:
					gService()->console().print( "It doesn't make any sense to join yourself.", sf::Color::Green );
					break;
				default:
#ifdef _DEBUG
					__debugbreak( );
#else
					__assume( 0 );
#endif
					break;
			}
		}
		
		if ( std::optional<std::string> userList( mNet.getByTag(TAGGED_REQ_USER_LIST_IN_LOBBY,
															   Online::Option::DEFAULT,
																-1) );
			std::nullopt != userList )
		{
			const std::string& _userList( userList.value() );
			const char* const ptr = _userList.data();
			const uint32_t userListSize = (uint32_t)_userList.size();
			uint32_t curPos = 0;
			// NOTE: Faster than std::set or std::unordered_set.
			std::vector<std::string> curUsers;
			while ( userListSize != curPos )
			{
				const uint8_t curSize = ptr[curPos];
				++curPos;
				const std::string otherNickname( _userList.substr(curPos, curSize) );
				curUsers.emplace_back( otherNickname );
				curPos += curSize;
			}
			for ( auto it = mUserList.begin(); mUserList.end() != it; )
			{
				bool isAlive = false;
				for ( auto it2 = curUsers.cbegin(); curUsers.cend() != it2; ++it2 )
				{
					if ( it->first == *it2 )
					{
						isAlive = true;
						curUsers.erase( it2 );
						break;
					}
				}
				if ( false == isAlive )
				{
					it = mUserList.erase(it);
				}
				else
				{
					++it;
				}
			}
			const sf::Vector2f center( mWindow_.getSize() );
			const sf::Vector2f offset( 20.f, 0.f );
			float mul = 0.f;
			for ( const std::string& nickname : curUsers )
			{
				sf::Text tf( nickname, mFont_ );
				tf.setPosition( center*.5f + offset*mul );
				mUserList.emplace( nickname, std::make_pair(tf, 0) );
				mul += 1.f;
			}
		}
	}
	
	if ( false == mIsReceiving )
	{
		mNet.receive( );
		mIsReceiving = true;
	}

	if ( UPDATE_USER_LIST_INTERVAL <= mFrameCount_update )
	{
		std::string req( TAGGED_REQ_USER_LIST_IN_LOBBY );
		mNet.send( req.data(), (int)req.size() );
	}

	if ( true == mTextInputBox.isActive() && 
		true == mTextInputBox.processEvent(eventQueue) )
	{
		joinRoom( mTextInputBox.inputString() );
	}

	if ( REQUEST_DELAY <= mFrameCount_requestDelay )
	{
		mFrameCount_requestDelay = 0;
	}

	Clock::time_point timeClickFirst = Clock::time_point::min();
	for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
	{
		if ( sf::Event::EventType::KeyPressed == it->type &&
			sf::Keyboard::Escape == it->key.code )
		{
			retVal = ::scene::online::ID::MAIN_MENU;
			it = eventQueue.erase(it);
		}
		else if ( sf::Event::EventType::MouseButtonPressed == it->type )
		{
			if ( sf::Mouse::Button::Left == it->mouseButton.button )
			{
				if ( Clock::time_point::min() == timeClickFirst )
				{
					timeClickFirst = Clock::now();
				}
				// When double-clicked,
				else if ( std::chrono::milliseconds(500) > Clock::now()-timeClickFirst )
				{
					createRoom( );
				}
				it = eventQueue.erase(it);
			}
			else if ( sf::Mouse::Button::Right == it->mouseButton.button )
			{
				mTextInputBox.activate( "What nickname has the one you want to play with?" );
				it = eventQueue.erase(it);
			}
		}
		else
		{
			++it;
		}
	}
	return retVal;
}

void scene::online::InLobby::draw( )
{
	mWindow_.draw( mBackground );
	if ( 0 != enteringRoom )
	{
		sf::Vector2f boxPos( mUserNicknamesBox.getPosition() );
		boxPos += sf::Vector2f(mAcceleration_.mComponents[0], mAcceleration_.mComponents[1]);
		mUserNicknamesBox.setPosition( boxPos );
		math::Vector<2> v( mDestination_.mComponents[0]-boxPos.x, mDestination_.mComponents[1]-boxPos.y );
		const float mag = v.magnitude();
		if ( mag < ERROR_RANGE )
		{
			enteringRoom *= 2;
		}
	}
	mWindow_.draw( mUserNicknamesBox );
	if ( UPDATE_USER_LIST_INTERVAL <= mFrameCount_update )
	{
		mFrameCount_update = 0;
		std::scoped_lock lock( MutexForGuideText );
		const uint8_t size = (uint8_t)mGuideTexts.size();
		if ( 0 != size )
		{
			if ( size <= ++mGuideTextIndex )
			{
				mGuideTextIndex = 0;
			}
			mGuideTextLabel.setString( mGuideTexts[mGuideTextIndex] );
		}
	}
	mWindow_.draw( mGuideTextLabel );
	uint8_t numOfMovingPoints = (uint8_t)mMovingPoints.size();
	for ( auto& pair : mUserList )
	{
		while ( true )
		{
			auto& tf = pair.second;
			const sf::Vector2f dir( mMovingPoints[tf.second] - tf.first.getPosition() );
			math::Vector<2> v( dir.x, dir.y );
			const float mag = v.magnitude();
			if ( mag <= ERROR_RANGE )
			{
				if ( numOfMovingPoints-1 == tf.second )
				{
					tf.second = 0;
				}
				else
				{
					++tf.second;
				}
			}
			else
			{
				const math::Vector<2> nv( v.normalize()*SPEED );
				tf.first.move( nv.mComponents[0], nv.mComponents[1] );
				mWindow_.draw( tf.first );
				break;
			}
		}
	}
	if ( true == mTextInputBox.isActive() )
	{
		mTextInputBox.draw( );
	}
	++mFrameCount_update;
	if ( 0 != mFrameCount_requestDelay )
	{
		++mFrameCount_requestDelay;
	}
}

void scene::online::InLobby::createRoom( )
{
	if ( 0 != mFrameCount_requestDelay )
	{
		gService()->console().print( "Retry to create a room later.", sf::Color::Green );
		return;
	}
	// Triggering
	mFrameCount_requestDelay = 1;
	std::string request( TAGGED_REQ_CREATE_ROOM );
	mNet.send( request.data(), (int)request.size() );
}

void scene::online::InLobby::_createRoom( const std::string_view& )
{
	createRoom( );
}

void scene::online::InLobby::joinRoom( const std::string_view& arg )
{
	if ( 0 != mFrameCount_requestDelay )
	{
		gService()->console().print( "Retry to join the room later.", sf::Color::Green );
		return;
	}
	// Exception
	if ( const size_t pos = arg.find_first_of(' ');
		arg.npos != pos	)
	{
		gService()->console().print( "User's nickname shouldn't have a space.", sf::Color::Green );
		return;
	}
	if ( arg == mNet.myNickname() )
	{
		gService()->console().print( "It doesn't make any sense to join yourself.", sf::Color::Green );
		return;
	}
	// Triggering
	mFrameCount_requestDelay = 1;
	Packet packet;
	std::string otherNickname( arg );
	packet.pack( TAGGED_REQ_JOIN_ROOM, otherNickname );
	mNet.send( packet );
}