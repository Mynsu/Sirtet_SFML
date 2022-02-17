#include "../../pch.h"
#include "InLobby.h"
#include "../../ServiceLocatorMirror.h"
#include "Online.h"

const uint16_t UPDATE_USER_LIST_INTERVAL = 300;
const uint16_t REQUEST_DELAY = 60;
const float ERROR_RANGE = 5.f;
const float SPEED_OF_NICKNAME_MOVE = .5f;
std::mutex MutexForGuideText, MutexForMovingPoints;

bool scene::online::InLobby::IsInstantiated = false;

scene::online::InLobby::InLobby( const sf::RenderWindow& window, ::scene::online::Online& net )
	: mIsReceiving( false ), mIsLoading( true ),
	mIndexForGuideText( 0 ), mRoomEntranceCase( 0 ),
	mFrameCountUserListUpdateInterval( 30 ), mFrameCountRequestDelay( 0 ),
	mNet( net ),
	mTextInputBox( window )
{
	ASSERT_TRUE( false == IsInstantiated );
	
	const sf::Vector2f winSize( window.getSize() );
	mBackground.setSize( winSize );
	mUsersBox.setSize( winSize );
	const std::string& myNickname = net.myNickname();
	auto pair = mUserList.emplace( std::piecewise_construct,
								  std::forward_as_tuple(myNickname),
								  std::forward_as_tuple(myNickname, mFont) );
	sf::Text& tf = pair.first->second.textFieldNickname;
	tf.setOrigin( 0.5f, 0.5f );
	tf.setPosition(	winSize*.5f );
	tf.setFillColor( sf::Color(0xffa500ff) ); // Orange
	loadResources( window );
#ifdef _DEV
	IServiceLocator* const service = gService();
	ASSERT_NOT_NULL( service );
	service->console().addCommand( CMD_CREATE_ROOM, std::bind(&scene::online::InLobby::_createRoom,
																	  this, std::placeholders::_1) );
	service->console().addCommand( CMD_JOIN_ROOM, std::bind(&scene::online::InLobby::joinRoom,
																		 this, std::placeholders::_1) );
#endif

	IsInstantiated = true;
}

scene::online::InLobby::~InLobby( )
{
#ifdef _DEV
	gService()->console().removeCommand( CMD_CREATE_ROOM );
	gService()->console().removeCommand( CMD_JOIN_ROOM );
#endif
	IsInstantiated = false;
}

void scene::online::InLobby::loadResources( const sf::RenderWindow& window )
{
	uint32_t backgroundColor = 0x29cdb5fa; // Cyan
	mDrawingInfo.usersBoxAnimationSpeed = 3.f;
	uint32_t myNicknameColor = 0xffa500ff;
	mDrawingInfo.nicknameFontSize = 30;
	mDrawingInfo.nicknameColor = 0xffffffff;
	mDrawingInfo.usersBoxPosition = ::math::Vector<2>(0.f, 100.f);
	mDrawingInfo.usersBoxSize = ::math::Vector<2>(800.f, 400.f);
	mDrawingInfo.usersBoxColor = sf::Color(0x000000ff);
	mDrawingInfo.usersBoxOutlineThickness = 5.f;
	mDrawingInfo.roomOutlineThickness = 11.f;
	mDrawingInfo.usersBoxOutlineColor = sf::Color(0x0000007f);
	mDrawingInfo.roomOutlineColor = sf::Color(0x3f3f3f7f);
	mDrawingInfo.movingPoints.clear( );
	mDrawingInfo.movingPoints.emplace_back( 0.f, 490.f );
	mDrawingInfo.movingPoints.emplace_back( 0.f, 110.f );
	mDrawingInfo.movingPoints.emplace_back( 600.f, 110.f );
	mDrawingInfo.movingPoints.emplace_back( 600.f, 490.f );
	sf::Vector2f guideTextLabelPosition( 400.f, 200.f );
	uint32_t guideTextColor = 0xffffff7f;
	mGuideTexts.clear( );
	mGuideTexts.emplace_back( "Double click to create a room." );
	mGuideTexts.emplace_back( "Right click to join a room." );
	uint16_t guideTextFontSize = 20;
	std::string fontPath( "Fonts/AGENCYB.TTF" );
	uint32_t shade = 0x0000007f;
	sf::Vector2f subWindowPosition( 300.f, 200.f );
	sf::Vector2f subWindowSize( 200.f, 100.f );
	uint32_t subWindowColor = 0xffff00ff;
	std::string subWindowTitleFont( "Fonts/AGENCYB.TTF" );
	sf::Vector2f subWindowTitleRelativePosition( 20.f, 10.f );
	uint16_t subWindowTitleFontSize = 30;
	uint32_t subWindowTitleFontColor = 0xffffffff;
	sf::Vector2f subWindowInputTextFieldRelativePosition( 20.f, 60.f );
	uint16_t subWindowInputTextFieldFontSize = 25;
	uint32_t subWindowInputTextFieldFontColor = 0xffffffff;
	mDrawingInfo.roomColor = sf::Color(0x3f3f3fff);
	mDrawingInfo.totalDistanceUsersBoxToRoom = 0.f;
	mDrawingInfo.remainingDistanceUsersBoxToRoom = 0.f;
	mSoundPaths[(int)SoundIndex::SELECTION] = "Sounds/selection.wav";
	mDrawingInfo.centerPosition = sf::Vector2f(window.getSize()/2u);
	
	//
	// From 'InLobby.lua'.
	//

	lua_State* lua = luaL_newstate();
	std::string scriptPath( "Scripts/InLobby.lua" );
	if ( true == luaL_dofile(lua, scriptPath.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL,
										   "File Not Found: "+scriptPath );
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
												   varName, scriptPath );
		}
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 varName, scriptPath );
		}
		lua_pop( lua, 1 );

		varName = "Font";
		lua_getglobal( lua, varName.data() );
		type = lua_type(lua, TOP_IDX);
		if ( LUA_TSTRING == type )
		{
			fontPath = lua_tostring(lua, TOP_IDX);
		}
		else if ( LUA_TNIL == type )
		{
			gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
												   varName, scriptPath );
		}
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
												   varName, scriptPath );
		}
		lua_pop( lua, 1 );

		varName = "BoxAnimationSpeed";
		lua_getglobal( lua, varName.data() );
		type = lua_type(lua, TOP_IDX);
		if ( LUA_TNUMBER == type )
		{
			mDrawingInfo.usersBoxAnimationSpeed = (float)lua_tonumber(lua, TOP_IDX);
		}
		else if ( LUA_TNIL == type )
		{
			gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
												   varName, scriptPath );
		}
		else
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 varName, scriptPath );
		}
		lua_pop( lua, 1 );

		std::string tableName( "Nickname" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
		}
		else
		{
			std::string field( "fontSize" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.nicknameFontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
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

			field = "colorForMine";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				myNicknameColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "colorForOthers";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.nicknameColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

		tableName = "Box";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
		}
		else
		{
			std::string field( "x" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.usersBoxPosition.mComponents[0] =
					(float)lua_tonumber(lua, TOP_IDX);
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

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.usersBoxPosition.mComponents[1] =
					(float)lua_tonumber(lua, TOP_IDX);
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

			field = "width";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.usersBoxSize.mComponents[0] =
					(float)lua_tonumber(lua, TOP_IDX);
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
			
			field = "height";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.usersBoxSize.mComponents[1] =
					(float)lua_tonumber(lua, TOP_IDX);
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

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.usersBoxColor = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
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

			field = "outlineThickness";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.usersBoxOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "outlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.usersBoxOutlineColor = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
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

		tableName = "MovingPoints";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
		}
		else
		{
			std::scoped_lock lock( MutexForMovingPoints );
			mDrawingInfo.movingPoints.clear();
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
					else if ( LUA_TNIL == type )
					{
						gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
															   tableName+":...", scriptPath );
					}
					else
					{
						gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
																 tableName+":...", scriptPath );
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
					else if ( LUA_TNIL == type )
					{
						gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
															   tableName+":...", scriptPath );
					}
					else
					{
						gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
																 tableName+":...", scriptPath );
					}
					mDrawingInfo.movingPoints.emplace_back( point );
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
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
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

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				guideTextLabelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				guideTextColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
				guideTextFontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
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

		tableName = "GuideTexts";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
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
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
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

			field = "x";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowPosition.x = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "width";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowSize.x = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "height";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowSize.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "font";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				subWindowTitleFont = lua_tostring(lua, TOP_IDX);
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

			field = "titleRelativeX";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowTitleRelativePosition.x = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "titleRelativeY";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowTitleRelativePosition.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "titleFontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowTitleFontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
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

			field = "titleFontColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowTitleFontColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "inputTextFieldX";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowInputTextFieldRelativePosition.x = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "inputTextFieldY";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowInputTextFieldRelativePosition.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "inputTextFieldFontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowInputTextFieldFontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
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

			field = "inputTextFieldFontColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				subWindowInputTextFieldFontColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
			std::string innerTableName( "onSelection" );
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

	mBackground.setFillColor( sf::Color(backgroundColor) );
	{
		auto it = mUserList.find(mNet.myNickname());
		ASSERT_TRUE( mUserList.end() != it );
		it->second.textFieldNickname.setFillColor( sf::Color(myNicknameColor) );
	}
	::math::Vector<2> dir( mDrawingInfo.usersBoxPosition-
								::math::Vector<2>(0.f, 0.f) );
	mDrawingInfo.accelerationUsersBoxLeftTop0 = dir.normalize();
	mDrawingInfo.accelerationUsersBoxLeftTop0 *=
		mDrawingInfo.usersBoxAnimationSpeed;

	float mag = dir.magnitude();
	float ratio = mDrawingInfo.usersBoxAnimationSpeed/mag;
	const sf::Vector2f winSize( window.getSize() );
	dir = mDrawingInfo.usersBoxSize - ::math::Vector<2>(winSize.x, winSize.y);
	mag = dir.magnitude();
	float animationSpeed = mag*ratio;
	mDrawingInfo.relativeAccelerationUserBoxRightBottom0 = dir.normalize();
	mDrawingInfo.relativeAccelerationUserBoxRightBottom0 *= animationSpeed;
	mUsersBox.setFillColor( mDrawingInfo.usersBoxColor );
	mUsersBox.setOutlineThickness( mDrawingInfo.usersBoxOutlineThickness );
	mUsersBox.setOutlineColor( mDrawingInfo.usersBoxOutlineColor );
	mFont.loadFromFile( fontPath );
	mTextLabelForGuide.setPosition( guideTextLabelPosition );
	mTextLabelForGuide.setFillColor( sf::Color(guideTextColor) );
	mTextLabelForGuide.setCharacterSize( guideTextFontSize );
	mTextLabelForGuide.setFont( mFont );
	mTextInputBox.setBackgroundColor( sf::Color(shade) );
	mTextInputBox.setPosition( subWindowPosition );
	mTextInputBox.setSize( subWindowSize );
	mTextInputBox.setColor( sf::Color(subWindowColor) );
	if ( false == mTextInputBox.loadFont(subWindowTitleFont) )
	{
		gService()->console().printScriptError(ExceptionType::FILE_NOT_FOUND,
											   subWindowTitleFont, scriptPath );
	}
	mTextInputBox.setTitleDimension( subWindowTitleRelativePosition, subWindowTitleFontSize );
	mTextInputBox.setTitleColor( sf::Color(subWindowTitleFontColor) );
	mTextInputBox.setInputTextFieldDimension( subWindowInputTextFieldRelativePosition,
											 subWindowInputTextFieldFontSize );
	mTextInputBox.setInputTextFieldColor( sf::Color(subWindowInputTextFieldFontColor) );
	for ( auto& pair : mUserList )
	{
		sf::Text& tf = pair.second.textFieldNickname;
		tf.setCharacterSize(mDrawingInfo.nicknameFontSize);
	}

	//
	// From 'InRoom.lua'.
	//

	mDrawingInfo.roomPosition.mComponents[0] = 100.f;
	mDrawingInfo.roomPosition.mComponents[1] = 0.f;
	float cellSize = 30.f;
	scriptPath = "Scripts/InRoom.lua";
	lua = luaL_newstate();
	if ( true == luaL_dofile(lua, scriptPath.data()) )
	{
		gService()->console().printFailure( FailureLevel::FATAL,
										   "File Not Found: "+scriptPath );
	}
	else
	{
		const int TOP_IDX = -1;

		std::string tableName( "PlayerPanel" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
		}
		else
		{
			std::string field( "x" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			int type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.roomPosition.mComponents[0] = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.roomPosition.mComponents[1] = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "cellSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				cellSize = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.roomColor = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
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

			field = "outlineThickness";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.roomOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "outlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.roomOutlineColor = sf::Color((uint32_t)lua_tointeger(lua, TOP_IDX));
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
	lua_close( lua );

	dir = mDrawingInfo.roomPosition - mDrawingInfo.usersBoxPosition;
	mDrawingInfo.accelerationUsersBoxLeftTopToRoom = dir.normalize();
	mDrawingInfo.accelerationUsersBoxLeftTopToRoom *= mDrawingInfo.usersBoxAnimationSpeed;
	mDrawingInfo.totalDistanceUsersBoxToRoom = dir.magnitude();
	mDrawingInfo.remainingDistanceUsersBoxToRoom =
		mDrawingInfo.totalDistanceUsersBoxToRoom;
	ratio = mDrawingInfo.usersBoxAnimationSpeed/mDrawingInfo.totalDistanceUsersBoxToRoom;
	mDrawingInfo.roomSize =
		math::Vector<2>(::model::stage::GRID_WIDTH*cellSize,
						::model::stage::GRID_HEIGHT*cellSize);
	dir = mDrawingInfo.roomSize - mDrawingInfo.usersBoxSize;
	mag = dir.magnitude();
	animationSpeed = ratio*mag;
	mDrawingInfo.relativeAccelerationUsersBoxRightBottomToRoom = dir.normalize();
	mDrawingInfo.relativeAccelerationUsersBoxRightBottomToRoom *= animationSpeed;
}

::scene::online::ID scene::online::InLobby::update( std::vector<sf::Event>& eventQueue,
												   ::scene::online::Online& net,
												   const sf::RenderWindow& )
{
	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	if ( 2 == mRoomEntranceCase )
	{
		retVal = ::scene::online::ID::IN_ROOM;
		return retVal;
	}
	else if ( -2 == mRoomEntranceCase )
	{
		retVal = ::scene::online::ID::IN_ROOM_AS_HOST;
		return retVal;
	}

	if ( true == net.hasReceived() )
	{
		mIsReceiving = false;
		if ( const std::optional<std::string> resultCreatingRoom( net.getByTag(TAGGED_REQ_CREATE_ROOM,
																				Online::Option::RETURNING_TAG_ATTACHED,
																				0) );
			std::nullopt != resultCreatingRoom )
		{
			mRoomEntranceCase = -1;
			mUserList.clear( );
		}
		else if ( const std::optional<std::string> resultJoiningRoom( net.getByTag(TAGGED_REQ_JOIN_ROOM,
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
					mRoomEntranceCase = 1;
					mUserList.clear( );
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
			}
		}
		
		if ( const std::optional<std::string> userList( net.getByTag(TAGGED_REQ_USER_LIST,
																	Online::Option::DEFAULT,
																	-1) );
			std::nullopt != userList )
		{
			const std::string& _userList( userList.value() );
			const char* const ptr = _userList.data();
			const uint16_t userListSize = (uint16_t)_userList.size();
			uint16_t curPos = 0;
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
			const sf::Vector2f offset( 20.f, 0.f );
			float mul = 0.f;
			for ( const std::string& nickname : curUsers )
			{
				auto pair = mUserList.emplace( std::piecewise_construct,
											  std::forward_as_tuple(nickname),
											  std::forward_as_tuple(nickname, mFont) );
				sf::Text& tf = pair.first->second.textFieldNickname;
				tf.setFillColor( sf::Color(mDrawingInfo.nicknameColor) );
				tf.setPosition( mDrawingInfo.centerPosition + offset*mul );
				tf.setCharacterSize( mDrawingInfo.nicknameFontSize );
				mul += 1.f;
			}
		}
	}
	
	if ( false == mIsReceiving )
	{
		net.receive( );
		mIsReceiving = true;
	}

	if ( UPDATE_USER_LIST_INTERVAL <= mFrameCountUserListUpdateInterval )
	{
		std::string request( TAGGED_REQ_USER_LIST );
		net.send( request.data(), (int)request.size() );
	}

	if ( true == mTextInputBox.isActive() && 
		true == mTextInputBox.processEvent(eventQueue) )
	{
		joinRoom( mTextInputBox.inputString() );
		if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::SELECTION]) )
		{
			gService()->console().printFailure(FailureLevel::WARNING,
											   "File Not Found: "+mSoundPaths[(int)SoundIndex::SELECTION] );
		}
	}

	if ( REQUEST_DELAY <= mFrameCountRequestDelay )
	{
		mFrameCountRequestDelay = 0;
	}

	eventQueue.erase(std::remove_if(eventQueue.begin(), eventQueue.end(), [this, &retVal](const sf::Event& event)->bool
									{
										bool isRemoved = false;
										if ( sf::Event::EventType::KeyPressed == event.type &&
											sf::Keyboard::Escape == event.key.code )
										{
											if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::SELECTION]) )
											{
												gService()->console().printFailure(FailureLevel::WARNING,
																				   "File Not Found: "+mSoundPaths[(int)SoundIndex::SELECTION] );
											}
											retVal = ::scene::online::ID::MAIN_MENU;
											isRemoved = true;
										}
										else if ( sf::Event::EventType::MouseButtonPressed == event.type )
										{
											if ( sf::Mouse::Button::Left == event.mouseButton.button )
											{
												const Clock::time_point now = Clock::now();
												const math::Vector<2> pos( (float)event.mouseButton.x, (float)event.mouseButton.y );
												const math::Vector<2> diff( pos - mLatestMouseEvent.clickPosition );
												if ( std::chrono::milliseconds(500) > now-mLatestMouseEvent.latestClickTime &&
													100.f > diff.magnitude() )
												{
													createRoom( );
													if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::SELECTION]) )
													{
														gService()->console().printFailure(FailureLevel::WARNING,
																						   "File Not Found: "+mSoundPaths[(int)SoundIndex::SELECTION] );
													}
												}
												mLatestMouseEvent.latestClickTime = now;
												mLatestMouseEvent.clickPosition = pos;
												isRemoved = true;
											}
											else if ( sf::Mouse::Button::Right == event.mouseButton.button )
											{
												mTextInputBox.activate( "What nickname has the one you want to play with?" );
												isRemoved = true;
											}
										}
										return isRemoved;
									}), eventQueue.end());
	return retVal;
}

void scene::online::InLobby::draw( sf::RenderWindow& window )
{
	window.draw( mBackground );
	if ( true == mIsLoading )
	{
		sf::Vector2f pos( mUsersBox.getPosition() );
		pos.x += mDrawingInfo.accelerationUsersBoxLeftTop0.mComponents[0];
		pos.y += mDrawingInfo.accelerationUsersBoxLeftTop0.mComponents[1];
		mUsersBox.setPosition( pos );
		const ::math::Vector<2> v( mDrawingInfo.usersBoxPosition.mComponents[0]-pos.x,
								  mDrawingInfo.usersBoxPosition.mComponents[1]-pos.y );
		if ( v.magnitude() < ERROR_RANGE )
		{
			mIsLoading = false;
		}
		sf::Vector2f size( mUsersBox.getSize() );
		size.x += mDrawingInfo.relativeAccelerationUserBoxRightBottom0.mComponents[0];
		size.y += mDrawingInfo.relativeAccelerationUserBoxRightBottom0.mComponents[1];
		mUsersBox.setSize( size );
	}
	else if ( 0 != mRoomEntranceCase )
	{
		sf::Vector2f boxPos( mUsersBox.getPosition() );
		boxPos += sf::Vector2f(mDrawingInfo.accelerationUsersBoxLeftTopToRoom.mComponents[0],
							   mDrawingInfo.accelerationUsersBoxLeftTopToRoom.mComponents[1]);
		mUsersBox.setPosition( boxPos );
		mDrawingInfo.remainingDistanceUsersBoxToRoom -= mDrawingInfo.usersBoxAnimationSpeed;
		sf::Vector2f boxSize( mUsersBox.getSize() );
		boxSize += sf::Vector2f(mDrawingInfo.relativeAccelerationUsersBoxRightBottomToRoom.mComponents[0],
								mDrawingInfo.relativeAccelerationUsersBoxRightBottomToRoom.mComponents[1]);
		mUsersBox.setSize( boxSize );
		const float ratio = mDrawingInfo.remainingDistanceUsersBoxToRoom/mDrawingInfo.totalDistanceUsersBoxToRoom;
		const float ratioComplement = 1.f-ratio;
		sf::Color blended;
		blended.r = (uint8_t)(mDrawingInfo.usersBoxColor.r*ratio +
							  mDrawingInfo.roomColor.r*ratioComplement);
		blended.g = (uint8_t)(mDrawingInfo.usersBoxColor.g*ratio +
							  mDrawingInfo.roomColor.g*ratioComplement);
		blended.b = (uint8_t)(mDrawingInfo.usersBoxColor.b*ratio +
							  mDrawingInfo.roomColor.b*ratioComplement);
		blended.a = (uint8_t)(mDrawingInfo.usersBoxColor.a*ratio +
							  mDrawingInfo.roomColor.a*ratioComplement);
		mUsersBox.setFillColor( blended );
		const float linearInterpolatedThickness =
			mDrawingInfo.usersBoxOutlineThickness*ratio + mDrawingInfo.roomOutlineThickness*(1.f-ratio);
		mUsersBox.setOutlineThickness( linearInterpolatedThickness );
		blended.r = (uint8_t)(mDrawingInfo.usersBoxOutlineColor.r*ratio +
							  mDrawingInfo.roomOutlineColor.r*ratioComplement);
		blended.g = (uint8_t)(mDrawingInfo.usersBoxOutlineColor.g*ratio +
							  mDrawingInfo.roomOutlineColor.g*ratioComplement);
		blended.b = (uint8_t)(mDrawingInfo.usersBoxOutlineColor.b*ratio +
							  mDrawingInfo.roomOutlineColor.b*ratioComplement);
		blended.a = (uint8_t)(mDrawingInfo.usersBoxOutlineColor.a*ratio +
							  mDrawingInfo.roomOutlineColor.a*ratioComplement);
		mUsersBox.setOutlineColor( blended );
		const math::Vector<2> v( mDrawingInfo.roomPosition.mComponents[0]-boxPos.x,
						  mDrawingInfo.roomPosition.mComponents[1]-boxPos.y );
		const float mag = v.magnitude();
		if ( mag < ERROR_RANGE )
		{
			mRoomEntranceCase *= 2;
		}
	}
	window.draw( mUsersBox );
	if ( UPDATE_USER_LIST_INTERVAL <= mFrameCountUserListUpdateInterval )
	{
		mFrameCountUserListUpdateInterval = 0;
		std::scoped_lock lock( MutexForGuideText );
		const uint8_t size = (uint8_t)mGuideTexts.size();
		if ( 0 != size )
		{
			if ( size <= ++mIndexForGuideText )
			{
				mIndexForGuideText = 0;
			}
			mTextLabelForGuide.setString( mGuideTexts[mIndexForGuideText] );
		}
	}
	if ( 0 == mRoomEntranceCase )
	{
		window.draw( mTextLabelForGuide );
	}
	{
		std::scoped_lock lock( MutexForMovingPoints );
		uint8_t numOfMovingPoints = (uint8_t)mDrawingInfo.movingPoints.size();
		for ( auto& pair : mUserList )
		{
			while ( true )
			{
				auto& textFieldAndDestination = pair.second;
				const sf::Vector2f dir( mDrawingInfo.movingPoints[textFieldAndDestination.destinationIndex] -
									   textFieldAndDestination.textFieldNickname.getPosition() );
				math::Vector<2> v( dir.x, dir.y );
				const float mag = v.magnitude();
				if ( mag <= ERROR_RANGE )
				{
					if ( numOfMovingPoints-1 == textFieldAndDestination.destinationIndex )
					{
						textFieldAndDestination.destinationIndex = 0;
					}
					else
					{
						++textFieldAndDestination.destinationIndex;
					}
				}
				else
				{
					const math::Vector<2> nv( v.normalize()*SPEED_OF_NICKNAME_MOVE );
					textFieldAndDestination.textFieldNickname.move( nv.mComponents[0], nv.mComponents[1] );
					window.draw( textFieldAndDestination.textFieldNickname );
					break;
				}
			}
		}
	}
	if ( true == mTextInputBox.isActive() )
	{
		mTextInputBox.draw( window );
	}
	++mFrameCountUserListUpdateInterval;
	if ( 0 != mFrameCountRequestDelay )
	{
		++mFrameCountRequestDelay;
	}
}

void scene::online::InLobby::createRoom( )
{
	if ( 0 != mFrameCountRequestDelay )
	{
		gService()->console().print( "Retry to create a room later.", sf::Color::Green );
		return;
	}
	// Triggering
	mFrameCountRequestDelay = 1;
	std::string request( TAGGED_REQ_CREATE_ROOM );
	mNet.send( request.data(), (int)request.size() );
}

void scene::online::InLobby::_createRoom( const std::string_view& )
{
	createRoom( );
}

void scene::online::InLobby::joinRoom( const std::string_view& arg )
{
	if ( 0 != mFrameCountRequestDelay )
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
	mFrameCountRequestDelay = 1;
	Packet packet;
	std::string otherNickname( arg );
	packet.pack( TAGGED_REQ_JOIN_ROOM, otherNickname );
	mNet.send( packet );
}