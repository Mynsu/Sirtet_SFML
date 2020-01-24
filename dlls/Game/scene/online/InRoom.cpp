#include "../../pch.h"
#include "InRoom.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "../VaultKeyList.h"
#include <utility>

const uint32_t EMPTY_SLOT = 0;
const uint32_t ROTATION_INTERVAL_THRESHOLD_MS = 1000;

bool scene::online::InRoom::IsInstantiated = false;

scene::online::InRoom::InRoom( sf::RenderWindow& window, Online& net, const bool asHost )
	: mIsReceiving( false ), mAsHost( asHost ),
	mIsPlaying_( false ), mIsMouseOverStartButton_( false ), mIsStartButtonPressed_( false ),
	mFrameCount_rotationInterval( 0 ),
	mWindow_( window ), mNet( net ),
	mOtherPlayerSlots{ EMPTY_SLOT }
{
	ASSERT_TRUE( false == IsInstantiated );

	mParticipants.reserve( ROOM_CAPACITY );
	mParticipants.emplace( mNet.myNicknameHashed(), Participant(mNet.myNickname(), ::ui::PlayView(mWindow_, mNet)) );
	IServiceLocator* const service = gService();
	ASSERT_NOT_NULL( service );
	service->console( ).addCommand( CMD_LEAVE_ROOM, std::bind( &scene::online::InRoom::_leaveRoom,
															  this, std::placeholders::_1 ) );
	if ( true == asHost )
	{
		service->console( ).addCommand( CMD_START_GAME, std::bind( &scene::online::InRoom::_startGame,
																	 this, std::placeholders::_1 ) );
	}
	loadResources( );

	IsInstantiated = true;
}

scene::online::InRoom::~InRoom( )
{
	if ( nullptr != gService() )
	{
		gService( )->console( ).removeCommand( CMD_LEAVE_ROOM );
		if ( true == mAsHost )
		{
			gService( )->console( ).removeCommand( CMD_START_GAME );
		}
	}

	IsInstantiated = false;
}

void scene::online::InRoom::loadResources( )
{
	uint32_t backgroundColor = 0x8ae5ff'ff;
	std::string fontPathNName( "Fonts/AGENCYR.TTF" );
	sf::Vector2f myPanelPosition( 100.0, 0.0 );
	uint32_t myPanelColor = 0x3f3f3f'ff;
	float myPanelOutlineThickness = 11.0;
	uint32_t myPanelOutlineColor = 0x3f3f3f'7f;
	float myStageCellSize = 30.0;
	std::string vfxComboPathNName( "Vfxs/Combo.png" );
	sf::Vector2i vfxComboClipSize( 256, 256 );
	sf::Vector2f nextTetriminoPanelPosition( 520.f, 30.f );
	float nextTetriminoPanelCellSize = 30.0;
	uint32_t nextTetriminoPanelColor = 0x000000'ff;
	float nextTetriminoPanelOutlineThickness = 5.0;
	uint32_t nextTetriminoPanelOutlineColor = 0x000000'7f;
	mDrawingInfo.theta_degree = 1.5f;
	mDrawingInfo.scaleFactor = 0.9f;
	mDrawingInfo.framesRotationInterval = 60;
	mDrawingInfo.position = sf::Vector2f(600.f, 400.f);
	mDrawingInfo.cellSize = 8.f;
	uint32_t otherPlayerPanelColor = 0x3f3f3f'ff;
	mDrawingInfo.panelColor_on = 0x3f3f3f'ff;
	float otherPlayerSlotMargin = 10.f;
	float otherPlayerPanelOutlineThickness = 1.f;
	uint32_t otherPlayerPanelOutlineColor = 0x0000007f;
	mDrawingInfo.outlineThickness_on = 5.f;
	mDrawingInfo.outlineColor_on = 0x0000007f;
	mDrawingInfo.countdownSpritePathNName = "Images/Countdown.png";
	mDrawingInfo.countdownSpriteClipSize = sf::Vector2i(256, 256);
	uint32_t nicknameFontSize = 10;
	uint32_t nicknameFontColor = 0xffffffff;
	sf::Vector2f startingGuidePosition( 550.f, 60.f );
	uint32_t startingGuideFontSize = 30;
	uint32_t startingGuideFontColor = 0xffffffff;

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/InRoom.lua" );
	if ( true == luaL_dofile(lua, scriptPathNName.data()) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL, "File Not Found: "+scriptPathNName );
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
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 varName, scriptPathNName );
		}
		lua_pop( lua, 1 );

		varName = "Font";
		lua_getglobal( lua, varName.data() );
		type = lua_type(lua, TOP_IDX);
		if ( LUA_TSTRING == type )
		{
			fontPathNName = lua_tostring(lua, TOP_IDX);
		}
		else if ( LUA_TNIL == type )
		{
			gService()->console().printScriptError( ExceptionType::VARIABLE_NOT_FOUND,
												   varName, scriptPathNName );
		}
		else
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 varName, scriptPathNName );
		}
		lua_pop( lua, 1 );

		std::string tableName( "PlayerPanel" );
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
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
				myPanelPosition.x = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				myPanelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "cellSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				myStageCellSize = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				myPanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "outlineThickness";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				myPanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "outlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				myPanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

		tableName = "CountdownSprite";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				mDrawingInfo.countdownSpritePathNName = lua_tostring(lua, TOP_IDX);
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

			field = "clipWidth";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.countdownSpriteClipSize.x = (int)lua_tointeger(lua, TOP_IDX);
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

			field = "clipHeight";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.countdownSpriteClipSize.y = (int)lua_tointeger(lua, TOP_IDX);
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

		tableName = "VfxCombo";
		lua_getglobal( lua, tableName.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				vfxComboPathNName = lua_tostring(lua, TOP_IDX);
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

			field = "clipWidth";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				vfxComboClipSize.x = (int)lua_tointeger(lua, TOP_IDX);
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

			field = "clipHeight";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				vfxComboClipSize.y = (int)lua_tointeger(lua, TOP_IDX);
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

		tableName = "NextTetriminoPanel";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "x" );
			field = "x";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				nextTetriminoPanelPosition.x = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				nextTetriminoPanelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "cellSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				nextTetriminoPanelCellSize = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				nextTetriminoPanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "outlineThickness";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				nextTetriminoPanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "outlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				nextTetriminoPanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "theta_degree";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.theta_degree = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "rotationIntervalMs";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				auto& vault = gService()->vault();
				if ( const auto it = vault.find(HK_FORE_FPS);
					vault.end() != it )
				{
					const uint32_t fps = it->second;
					const uint32_t rotationIntervalMs =	(uint32_t)lua_tointeger(lua, TOP_IDX);
					mDrawingInfo.framesRotationInterval = fps*rotationIntervalMs/1000;
				}
#ifdef _DEBUG
				else
				{
					__debugbreak( );
				}
#endif
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

			field = "scaleFactor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.scaleFactor = (float)lua_tonumber(lua, TOP_IDX);
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

		tableName = "OtherPlayerSlot";
		lua_getglobal( lua, tableName.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "x" );
			field = "x";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.position.x = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.position.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "cellSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.cellSize = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "margin";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				otherPlayerSlotMargin = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "color";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				otherPlayerPanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "color_on";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.panelColor_on = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "outlineThickness";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				otherPlayerPanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "outlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				otherPlayerPanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "outlineThickness_on";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.outlineThickness_on = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "outlineColor_on";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.outlineColor_on = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "nicknameFontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				nicknameFontSize = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "nicknameFontColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				nicknameFontColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

		tableName = "StartingGuide";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
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
				startingGuidePosition.x = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "y";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				startingGuidePosition.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "fontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				startingGuideFontSize = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "fontColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				startingGuideFontColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
	lua_close( lua );

	const uint32_t cellOutlineColor = 0x0000007f;
	mBackground.setSize( sf::Vector2f(mWindow_.getSize()) );
	mBackground.setFillColor( sf::Color(backgroundColor) );
	if ( auto it = mParticipants.find(mNet.myNicknameHashed());
		mParticipants.end() != it )
	{
		::ui::PlayView& playView = it->second.playView;
		::model::Stage& stage = playView.stage();
		stage.setPosition( myPanelPosition );
		stage.setSize( myStageCellSize );
		stage.setBackgroundColor( sf::Color(myPanelColor),
								 myPanelOutlineThickness, sf::Color(myPanelOutlineColor),
								 sf::Color(cellOutlineColor) );
		if ( false == playView.loadCountdownSprite(mDrawingInfo.countdownSpritePathNName) )
		{
			// File Not Found Exception
			gService( )->console( ).printScriptError( ExceptionType::FILE_NOT_FOUND,
													"Path:CountdownSprite", scriptPathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
		}
		playView.setCountdownSpriteDimension( myPanelPosition, myStageCellSize, mDrawingInfo.countdownSpriteClipSize );
		::model::Tetrimino& tetrimino =	playView.currentTetrimino();
		tetrimino.setOrigin( myPanelPosition );
		tetrimino.setSize( myStageCellSize );
		::vfx::Combo& vfxCombo = playView.vfxCombo();
		vfxCombo.setOrigin( myPanelPosition, myStageCellSize, vfxComboClipSize );
		::ui::NextTetriminoPanel& nextTetPanel = playView.nextTetriminoPanel();
		nextTetPanel.setDimension( nextTetriminoPanelPosition, nextTetriminoPanelCellSize );
		mNextTetriminoPanelBound = nextTetPanel.globalBounds();
		mNextTetriminoPanelBound.width += 5.f;
		mNextTetriminoPanelBound.height += 5.f;
		nextTetPanel.setBackgroundColor( sf::Color(nextTetriminoPanelColor),
										nextTetriminoPanelOutlineThickness, sf::Color(nextTetriminoPanelOutlineColor),
										sf::Color(cellOutlineColor) );
		if ( false == vfxCombo.loadResources(vfxComboPathNName) )
		{
			gService( )->console( ).printFailure( FailureLevel::FATAL,
												 "File Not Found: "+vfxComboPathNName );
#ifdef _DEBUG
			__debugbreak( );
#endif
		}
	}
	// Exception
#ifdef _DEBUG
	else
	{
		__debugbreak( );
	}
#endif
	mDrawingInfo.positionDifferences[0] =
		sf::Vector2f(0.f, mDrawingInfo.cellSize*::model::stage::GRID_HEIGHT+otherPlayerSlotMargin);
	mDrawingInfo.positionDifferences[1] =
		sf::Vector2f(mDrawingInfo.cellSize*::model::stage::GRID_WIDTH+otherPlayerSlotMargin, 0.f);
	mDrawingInfo.positionDifferences[2] =
		sf::Vector2f(0.f, 0.f);
	const sf::Vector2f stageSize( ::model::stage::GRID_WIDTH*mDrawingInfo.cellSize,
							::model::stage::GRID_HEIGHT*mDrawingInfo.cellSize );
	mOtherPlayerSlotBackground.setSize( stageSize );
	mOtherPlayerSlotBackground.setFillColor( sf::Color(otherPlayerPanelColor) );
	mOtherPlayerSlotBackground.setOutlineThickness( otherPlayerPanelOutlineThickness );
	mOtherPlayerSlotBackground.setOutlineColor( sf::Color(otherPlayerPanelOutlineColor) );
	mNicknameLabel.setCharacterSize( nicknameFontSize );
	mNicknameLabel.setFillColor( sf::Color(nicknameFontColor) );
	if ( false == mFont.loadFromFile(fontPathNName) )
	{
		gService( )->console( ).printFailure( FailureLevel::FATAL,
											 "File Not Found: "+fontPathNName );
	}
	mNicknameLabel.setFont( mFont );
	mStartingGuide.setString( "Start" );
	mStartingGuide.setFont( mFont );
	mStartingGuide.setCharacterSize( startingGuideFontSize );
	const sf::FloatRect bound( mStartingGuide.getLocalBounds() );
	const sf::Vector2f center( bound.width*0.5f, bound.height*0.5f );
	mStartingGuide.setOrigin( center );
	mStartingGuide.setPosition( startingGuidePosition );
	mStartingGuide.setFillColor( sf::Color(startingGuideFontColor) );

	::model::Tetrimino::LoadResources( );
}

::scene::online::ID scene::online::InRoom::update( std::list<sf::Event>& eventQueue )
{
	::scene::online::ID nextSceneID = ::scene::online::ID::AS_IS;
	const HashedKey myNicknameHashed = mNet.myNicknameHashed();
	if ( true == mNet.hasReceived() )
	{
		mIsReceiving = false;
		if ( std::optional<std::string> userList( mNet.getByTag(TAGGED_NOTI_UPDATE_USER_LIST,
															   Online::Option::DEFAULT,
															   -1) );
			std::nullopt != userList )
		{
			// 궁금: 왜 8KB 버퍼가 다 차지?
			const std::string& _userList( userList.value() );
			const uint32_t userListSize = (uint32_t)_userList.size();
			const char* const ptr = _userList.data();
			// 궁금: 최적화할 여지
			std::unordered_map<HashedKey, std::string> users;
			uint32_t curPos = 0;
			while ( userListSize != curPos )
			{
				const uint8_t curSize = ptr[curPos];
				++curPos;
				const std::string otherNickname( _userList.substr(curPos, curSize) );
				const HashedKey otherNicknameHashed = ::util::hash::Digest( otherNickname.data(),
																	(uint8_t)otherNickname.size() );
				users.emplace( otherNicknameHashed, otherNickname );
				curPos += curSize;
			}
			for ( auto it = mParticipants.begin(); mParticipants.end() != it; )
			{
				if ( users.end() == users.find(it->first) )
				{
#ifdef _DEBUG
					if ( it->first == myNicknameHashed )
					{
						__debugbreak( );
					}
#endif
					for ( HashedKey& nicknameHashed : mOtherPlayerSlots )
					{
						if ( it->first == nicknameHashed )
						{
							nicknameHashed = EMPTY_SLOT;
							break;
						}
					}
					it = mParticipants.erase(it);
					//TODO: 누가 종료했다고 알리기.
				}
				else
				{
					users.erase( it->first );
					++it;
				}
			}
			for ( const auto& pair : users )
			{
				mParticipants.emplace( pair.first, Participant(pair.second, ::ui::PlayView(mWindow_, mNet, false)) );
				uint8_t slotIdx = 0;
				while ( ROOM_CAPACITY-1 != slotIdx )
				{
					if ( EMPTY_SLOT == mOtherPlayerSlots[slotIdx] )
					{
						mOtherPlayerSlots[slotIdx] = pair.first;
						break;
					}
					++slotIdx;
				}
#ifdef _DEBUG
				if ( ROOM_CAPACITY-1 == slotIdx )
				{
					__debugbreak( );
				}
#endif
				::ui::PlayView& playView = mParticipants[pair.first].playView;
				::model::Stage& stage =	playView.stage();
				// TODO: 닉네임 보여주기
				const sf::Vector2f pos( mDrawingInfo.position-mDrawingInfo.positionDifferences[slotIdx] );
				stage.setPosition( pos );
				stage.setSize( mDrawingInfo.cellSize );
				const sf::Color cellOutlineColor( 0x000000'7f );
				// TODO: parameter 줄이기
				stage.setBackgroundColor( sf::Color(mDrawingInfo.panelColor_on),
										 mDrawingInfo.outlineThickness_on, sf::Color(mDrawingInfo.outlineColor_on),
										 cellOutlineColor );
				::model::Tetrimino& curTet = playView.currentTetrimino();
				curTet.setOrigin( pos );
				curTet.setSize( mDrawingInfo.cellSize );
				if ( false == playView.loadCountdownSprite(mDrawingInfo.countdownSpritePathNName) )
				{
					gService()->console().printFailure( FailureLevel::WARNING,
														"Can't find a countdown sprite." );
#ifdef _DEBUG
					__debugbreak( );
#endif
				}
				playView.setCountdownSpriteDimension( pos,
													mDrawingInfo.cellSize,
													 mDrawingInfo.countdownSpriteClipSize );
			}
		}
		if ( const std::optional<std::string> response( mNet.getByTag(TAGGED_REQ_LEAVE_ROOM,
																	  Online::Option::RETURN_TAG_ATTACHED,
																	  NULL) );
			std::nullopt != response )
		{
			nextSceneID = ::scene::online::ID::IN_LOBBY;
			return nextSceneID;
		}
		// TODO: 이거 없애거나 이름 바꾸기.
		if ( false == mIsPlaying_ )
		{
			if ( std::optional<std::string> response( mNet.getByTag(TAGGED_REQ_GET_READY,
																   Online::Option::RETURN_TAG_ATTACHED,
																   NULL) );
				std::nullopt != response )
			{
				for ( auto& it : mParticipants )
				{
					it.second.playView.start( );
				}
				mIsPlaying_ = true;
			}
		}
		else
		{
			if ( std::optional<std::string> newCurrentTetriminos( mNet.getByTag(TAG_NEW_CURRENT_TETRIMINOS,
																			Online::Option::DEFAULT,
																			   -1) );
				std::nullopt != newCurrentTetriminos )
			{
				const std::string& newCurTetTypes = newCurrentTetriminos.value();
				const char* ptr = newCurTetTypes.data();
				const uint32_t totalSize = (uint32_t)newCurTetTypes.size();
				uint32_t curPos = 0;
				while ( totalSize != curPos )
				{
					const HashedKey nicknameHashed = ::ntohl(*(HashedKey*)&ptr[curPos]);
					curPos += sizeof(HashedKey);
					const ::model::tetrimino::Type newType = (::model::tetrimino::Type)ptr[curPos++];
					if ( const auto it = mParticipants.find(nicknameHashed);
						mParticipants.end() != it )
					{
						it->second.playView.setNewCurrentTetrimino( newType );
					}
#ifdef _DEBUG
					else
					{
						__debugbreak( );
					}
#endif
				}
			}

			if ( std::optional<std::string> currentTetriminosMove( mNet.getByTag(TAG_CURRENT_TETRIMINOS_MOVE,
																				 Online::Option::DEFAULT,
																				 -1) );
				std::nullopt != currentTetriminosMove )
			{
				std::string& curTetsMove = currentTetriminosMove.value();
				const char* ptr = curTetsMove.data();
				const uint32_t totalSize = (uint32_t)curTetsMove.size();
				uint32_t curPos = 0;
				while ( totalSize != curPos )
				{
					const HashedKey nicknameHashed = ::ntohl(*(HashedKey*)&ptr[curPos]);
					if ( nicknameHashed == myNicknameHashed )
					{
						curPos += sizeof(HashedKey) + sizeof(uint8_t) + sizeof(sf::Vector2<int8_t>);
						continue;
					}
					curPos += sizeof(HashedKey);
					const ::model::tetrimino::Rotation rotID = (::model::tetrimino::Rotation)ptr[curPos];
					++curPos;
					const sf::Vector2<int8_t> pos( *(sf::Vector2<int8_t>*)&ptr[curPos] );
					curPos += sizeof(sf::Vector2<int8_t>);
					if ( const auto it = mParticipants.find(nicknameHashed);
						mParticipants.end() != it )
					{
						::model::Tetrimino& tet = it->second.playView.currentTetrimino();
						tet.move( rotID, pos );
					}
#ifdef _DEBUG
					else
					{
						__debugbreak( );
					}
#endif
				}
			}

			if ( std::optional<std::string> stages( mNet.getByTag(TAG_STAGES,
																Online::Option::DEFAULT,
																  -1) );
				std::nullopt != stages )
			{
				std::string& _stages = stages.value();
				const char* ptr = _stages.data();
				const uint32_t totalSize = (uint32_t)_stages.size();
				uint32_t curPos = 0;
				while ( totalSize != curPos )
				{
					const HashedKey nicknameHashed = ::ntohl(*(HashedKey*)&ptr[curPos]);
					if ( nicknameHashed == myNicknameHashed )
					{
						curPos += sizeof(HashedKey) + sizeof(::model::stage::Grid);
						continue;
					}
					curPos += sizeof(HashedKey);
					const ::model::stage::Grid* const grid = (::model::stage::Grid*)&ptr[curPos];
					curPos += sizeof(::model::stage::Grid);
					if ( const auto it = mParticipants.find(nicknameHashed);
						mParticipants.end() != it )
					{
						::model::Stage& stage =	it->second.playView.stage();
						stage.deserialize( grid );
					}
#ifdef _DEBUG
					else
					{
						__debugbreak( );
					}
#endif
				}
			}

			if ( std::optional<std::string> numsOfLinesCleared( mNet.getByTag(TAG_NUMS_OF_LINES_CLEARED,
																			Online::Option::DEFAULT,
																			  -1) );
				std::nullopt != numsOfLinesCleared )
			{
				std::string& _numsOfLinesCleared = numsOfLinesCleared.value();
				const char* ptr = _numsOfLinesCleared.data();
				const uint32_t totalSize = (uint32_t)_numsOfLinesCleared.size();
				uint32_t curPos = 0;
				while ( totalSize != curPos )
				{
					const HashedKey nicknameHashed = ::ntohl(*(HashedKey*)&ptr[curPos]);
					curPos += sizeof(HashedKey);
					const uint8_t numOfLinesCleared = (uint8_t)ptr[curPos];
					++curPos;
					if ( const auto it = mParticipants.find(nicknameHashed);
						mParticipants.end() != it )
					{
						it->second.playView.setNumOfLinesCleared( numOfLinesCleared );
					}
#ifdef _DEBUG
					else
					{
						__debugbreak( );
					}
#endif
				}
			}

			if ( std::optional<std::string> gamesOver( mNet.getByTag(TAG_GAMES_OVER,
																	Online::Option::DEFAULT,
																	 -1) );
				std::nullopt != gamesOver )
			{
				std::string& _gamesOver = gamesOver.value();
				const uint32_t totalSize = (uint32_t)_gamesOver.size();
				const char* ptr = _gamesOver.data();
				uint32_t curPos = 0;
				while ( totalSize != curPos )
				{
					const HashedKey nicknameHashed = ::ntohl(*(HashedKey*)&ptr[curPos]);
					curPos += sizeof(HashedKey);
					if ( const auto it = mParticipants.find(nicknameHashed);
						mParticipants.end() != it )
					{
						it->second.playView.gameOver();
					}
#ifdef _DEBUG
					else
					{
						__debugbreak( );
					}
#endif
				}
			}

			if ( std::optional<std::string> allOver( mNet.getByTag(TAG_ALL_OVER,
																	Online::Option::RETURN_TAG_ATTACHED,
																	NULL) );
					 std::nullopt != allOver )
			{
				for ( auto& pair : mParticipants )
				{
					pair.second.playView.gameOver( );
				}
				mIsPlaying_ = false;
			}
		}
	}
	
	for ( auto& pair : mParticipants )
	{
		pair.second.playView.update( eventQueue );
	}

	if ( false == mIsReceiving )
	{
		mNet.receive( );
		mIsReceiving = true;
	}

	for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
	{
		//궁금: 계속 눌러도 인식하나?
		if ( sf::Event::EventType::MouseButtonPressed == it->type &&
			sf::Mouse::Button::Left == it->mouseButton.button &&
			true == mIsMouseOverStartButton_ )
		{
			mIsStartButtonPressed_ = true;
			it = eventQueue.erase(it);
		}
		else if ( sf::Event::EventType::MouseButtonReleased == it->type &&
			sf::Mouse::Button::Left == it->mouseButton.button &&
			true == mIsMouseOverStartButton_ )
		{
			startGame( );
			mIsStartButtonPressed_ = false;
			it = eventQueue.erase(it);
		}
		else if ( sf::Event::EventType::KeyPressed == it->type &&
				 sf::Keyboard::Escape == it->key.code )
		{
			leaveRoom( );
			it = eventQueue.erase(it);
		}
		else
		{
			++it;
		}
	}

	return nextSceneID;
}

void scene::online::InRoom::draw( )
{
	mWindow_.draw( mBackground );
	const HashedKey myNicknameHashed = mNet.myNicknameHashed();
	if ( const auto it = mParticipants.find(myNicknameHashed);
		mParticipants.end() != it )
	{
		if ( true == mAsHost )
		{
			::ui::NextTetriminoPanel& nextTetPanel = it->second.playView.nextTetriminoPanel();
			const sf::Vector2f mousePos( sf::Mouse::getPosition()-mWindow_.getPosition() );
			if ( true == mNextTetriminoPanelBound.contains(mousePos) )
			{
				if ( false == mIsMouseOverStartButton_ )
				{
					mStartingGuide.setString( "!" );
					const sf::FloatRect bound( mStartingGuide.getLocalBounds() );
					const sf::Vector2f center( bound.width*0.5f, bound.height*0.5f );
					mStartingGuide.setOrigin( center );
				}
				mIsMouseOverStartButton_ = true;
				if ( 0 == mFrameCount_rotationInterval )
				{
					nextTetPanel.rotate( mDrawingInfo.theta_degree );
					float degree = nextTetPanel.rotation();
					while ( 90.f <= degree )
					{
						degree -= 90.f;
					}
					if ( degree < mDrawingInfo.theta_degree/2.f )
					{
						mFrameCount_rotationInterval = 1;
					}
				}
				else if ( mDrawingInfo.framesRotationInterval < ++mFrameCount_rotationInterval )
				{
					mFrameCount_rotationInterval = 0;
				}
				if ( true == mIsStartButtonPressed_ )
				{
					nextTetPanel.scale( mDrawingInfo.scaleFactor );
					mIsStartButtonPressed_ = false;
				}
			}
			else
			{
				if ( true == mIsMouseOverStartButton_ )
				{
					mStartingGuide.setString( "Start" );
					const sf::FloatRect bound( mStartingGuide.getLocalBounds() );
					const sf::Vector2f center( bound.width*0.5f, bound.height*0.5f );
					mStartingGuide.setOrigin( center );
					nextTetPanel.resetScale( );
					nextTetPanel.resetRotation( );
					mFrameCount_rotationInterval = 0;
				}
				mIsMouseOverStartButton_ = false;
			}
		}
		it->second.playView.draw();
		if ( false == mIsPlaying_ )
		{
			mWindow_.draw( mStartingGuide );
		}
		const sf::Vector2f margin( 5.f, 0.f );
		mNicknameLabel.setPosition( it->second.playView.stage().position() + margin );
		mNicknameLabel.setString( mNet.myNickname() );
		mWindow_.draw( mNicknameLabel );
	}
#ifdef _DEBUG
	else
	{
		__debugbreak( );
	}
#endif
	
	for ( uint8_t i = 0; ROOM_CAPACITY-1 != i; ++i )
	{
		const sf::Vector2f pos( mDrawingInfo.position-mDrawingInfo.positionDifferences[i] );
		const sf::Vector2f margin( 5.f, 0.f );
		if ( EMPTY_SLOT == mOtherPlayerSlots[i] )
		{
			mOtherPlayerSlotBackground.setPosition( pos );
			mWindow_.draw( mOtherPlayerSlotBackground );
		}
		else
		{
			if ( const auto it = mParticipants.find(mOtherPlayerSlots[i]);
				mParticipants.end() != it )
			{
				it->second.playView.draw( );
				mNicknameLabel.setPosition( pos + margin );
				mNicknameLabel.setString( it->second.nickname );
				mWindow_.draw( mNicknameLabel );
			}
#ifdef _DEBUG
			else
			{
				__debugbreak( );
			}
#endif
		}
	}
}

void scene::online::InRoom::startGame()
{
	std::string request( TAGGED_REQ_START_GAME );
	mNet.send( request.data(), (int)request.size() );
}

void scene::online::InRoom::_startGame( const std::string_view& arg )
{
	startGame( );
}

void scene::online::InRoom::leaveRoom()
{
	std::string request( TAGGED_REQ_LEAVE_ROOM );
	mNet.send( request.data(), (int)request.size() );
}

void scene::online::InRoom::_leaveRoom( const std::string_view& )
{
	leaveRoom( );
}
