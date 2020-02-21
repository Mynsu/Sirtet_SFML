#include "../../pch.h"
#include "InRoom.h"
#include <Lib/Common.h>
#include <Lib/VaultKeyList.h>
#include <Lib/CommandList.h>
#include "Online.h"
#include "../../ServiceLocatorMirror.h"

const uint8_t NULL_EMPTY_SLOT = 0;
const uint16_t ROTATION_INTERVAL_THRESHOLD_MS = 1000;
const uint16_t ALL_OVER_FREEZE_MS = 2000;

bool scene::online::InRoom::IsInstantiated = false;

scene::online::Participant::Participant( const std::string& nickname, ::ui::PlayView&& playView )
// NOTE: r-value reference malfunctions within SFML, thus 'playView' is copied here.
	: nickname( nickname ), playView( playView )
{ }

scene::online::InRoom::InRoom( sf::RenderWindow& window, Online& net, const bool asHost )
	: mIsReceiving( false ), mAsHost( asHost ),
	mIsStartingGuideVisible_( true ),
	mIsMouseOverStartButton_( false ), mIsStartButtonPressed_( false ),
	mFrameCountCoolToRotateStartButton( 0 ),
	mMyNicknameHashed( net.myNicknameHashed() ), mMyNickname( net.myNickname() ),
	mNet( net ),
	mAlarms{ Clock::time_point::max() }, mOtherPlayerSlots{ NULL_EMPTY_SLOT }
{
	ASSERT_TRUE( false == IsInstantiated );
	
	mParticipants.reserve( ROOM_CAPACITY );
	mParticipants.emplace( net.myNicknameHashed(), Participant(net.myNickname(), ::ui::PlayView()) );
	IServiceLocator* const service = gService();
	ASSERT_NOT_NULL( service );
	service->console().addCommand( CMD_LEAVE_ROOM, std::bind( &scene::online::InRoom::_leaveRoom,
															  this, std::placeholders::_1 ) );
	if ( true == asHost )
	{
		service->console().addCommand( CMD_START_GAME, std::bind( &scene::online::InRoom::_startGame,
																	 this, std::placeholders::_1 ) );
	}
	loadResources( window );

	IsInstantiated = true;
}

scene::online::InRoom::~InRoom( )
{
	if ( nullptr != gService() )
	{
		gService()->console().removeCommand( CMD_LEAVE_ROOM );
		if ( true == mAsHost )
		{
			gService()->console().removeCommand( CMD_START_GAME );
		}
	}

	IsInstantiated = false;
}

void scene::online::InRoom::loadResources( sf::RenderWindow& window )
{
	uint32_t backgroundColor = 0x8ae5ff'ff;
	std::string fontPath( "Fonts/AGENCYR.TTF" );
	sf::Vector2f myPanelPosition( 100.0, 0.0 );
	uint32_t myPanelColor = 0x1f1f1f'ff;
	float myPanelOutlineThickness = 11.0;
	uint32_t myPanelOutlineColor = 0x3f3f3f'7f;
	float myStageCellSize = 30.0;
	mDrawingInfo.cellOutlineColor = 0x0000007f;
	mDrawingInfo.myNicknameFontSize = 30;
	mDrawingInfo.myNicknameColor = 0xffa500ff; // Orange
	mDrawingInfo.otherPlayerNicknameFontSize = 10;
	mDrawingInfo.otherPlayerNicknameFontColor = 0xffffffff;
	std::string vfxComboPath( "Vfxs/Combo.png" );
	sf::Vector2i vfxComboClipSize( 256, 256 );
	sf::Vector2f nextTetriminoPanelPosition( 520.f, 30.f );
	float nextTetriminoPanelCellSize = 30.0;
	uint32_t nextTetriminoPanelColor = 0x1f1f1f'ff;
	float nextTetriminoPanelOutlineThickness = 5.0;
	uint32_t nextTetriminoPanelOutlineColor = 0x3f3f3f'7f;
	mDrawingInfo.angularVelocity = 1.5f;
	mDrawingInfo.arcLength = 90;
	mDrawingInfo.scaleFactor = 0.9f;
	mDrawingInfo.framesRotationInterval = 60;
	mDrawingInfo.position = sf::Vector2f(600.f, 400.f);
	mDrawingInfo.cellSize = 8.f;
	uint32_t otherPlayerPanelColor = 0x1f1f1f'7f;
	mDrawingInfo.panelColor_on = 0x1f1f1f'ff;
	float otherPlayerSlotMargin = 10.f;
	float otherPlayerPanelOutlineThickness = 1.f;
	uint32_t otherPlayerPanelOutlineColor = 0x3f3f3f'7f;
	mDrawingInfo.outlineThickness_on = 5.f;
	mDrawingInfo.outlineColor_on = 0x3f3f3f'7f;
	mDrawingInfo.countdownSpritePath = "Images/Countdown.png";
	mDrawingInfo.countdownSpriteClipSize = sf::Vector2i(256, 256);
	sf::Vector2f startingGuidePosition( 550.f, 60.f );
	uint16_t startingGuideFontSize = 30;
	uint32_t startingGuideFontColor = 0xffffffff;
	mSoundPaths[(int)SoundIndex::ON_SELECTION] = "Sounds/selection.wav";
	mSoundPaths[(int)SoundIndex::GAME_OVER] = "Sounds/gameOver.wav";

	lua_State* lua = luaL_newstate();
	const std::string scriptPath( "Scripts/InRoom.lua" );
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

		varName = "CellOutlineColor";
		lua_getglobal( lua, varName.data() );
		type = lua_type(lua, TOP_IDX);
		if ( LUA_TNUMBER == type )
		{
			mDrawingInfo.cellOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				myPanelPosition.x = (float)lua_tonumber(lua, TOP_IDX);
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
				myPanelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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
				myStageCellSize = (float)lua_tonumber(lua, TOP_IDX);
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
				myPanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
				myPanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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
				myPanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "nicknameFontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.myNicknameFontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
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

			field = "nicknameFontColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.myNicknameColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

		tableName = "CountdownSprite";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				mDrawingInfo.countdownSpritePath = lua_tostring(lua, TOP_IDX);
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
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

		tableName = "VfxCombo";
		lua_getglobal( lua, tableName.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TSTRING == type )
			{
				vfxComboPath = lua_tostring(lua, TOP_IDX);
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														tableName+':'+field, scriptPath );
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

		tableName = "NextTetriminoPanel";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
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
				nextTetriminoPanelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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
				nextTetriminoPanelCellSize = (float)lua_tonumber(lua, TOP_IDX);
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
				nextTetriminoPanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
				nextTetriminoPanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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
				nextTetriminoPanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "angularVelocity";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.angularVelocity = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "arcLength";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.arcLength = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "rotationIntervalMs";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				auto& vault = gService()->vault();
				{
					const auto it = vault.find(HK_FORE_FPS);
					ASSERT_TRUE( vault.end() != it );
					const uint16_t fps = (uint16_t)it->second;
					const uint16_t rotationIntervalMs =	(uint16_t)lua_tointeger(lua, TOP_IDX);
					mDrawingInfo.framesRotationInterval = fps*rotationIntervalMs/1000;
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

		tableName = "OtherPlayerSlot";
		lua_getglobal( lua, tableName.data() );
		// Type Check Exception
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPath );
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
				mDrawingInfo.position.y = (float)lua_tonumber(lua, TOP_IDX);
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
				mDrawingInfo.cellSize = (float)lua_tonumber(lua, TOP_IDX);
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
				otherPlayerPanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
				otherPlayerPanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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
				otherPlayerPanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPath );
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
													   tableName+':'+field, scriptPath );
			}
			else
			{
				gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
														 tableName+':'+field, scriptPath );
			}
			lua_pop( lua, 1 );

			field = "nicknameFontSize";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.otherPlayerNicknameFontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
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

			field = "nicknameFontColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.otherPlayerNicknameFontColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

		tableName = "StartingGuide";
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
				startingGuidePosition.x = (float)lua_tonumber(lua, TOP_IDX);
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
				startingGuidePosition.y = (float)lua_tonumber(lua, TOP_IDX);
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
				startingGuideFontSize = (uint16_t)lua_tointeger(lua, TOP_IDX);
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
					mSoundPaths[(int)SoundIndex::ON_SELECTION] = lua_tostring(lua, TOP_IDX);
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

			innerTableName = "gameOver";
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
					mSoundPaths[(int)SoundIndex::GAME_OVER] = lua_tostring(lua, TOP_IDX);
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

	mBackground.setSize( sf::Vector2f(window.getSize()) );
	mBackground.setFillColor( sf::Color(backgroundColor) );
	{
		auto it = mParticipants.find(mMyNicknameHashed);
		ASSERT_TRUE( mParticipants.end() != it );
		::ui::PlayView& playView = it->second.playView;
		::model::Stage& stage = playView.stage();
		stage.setPosition( myPanelPosition );
		stage.setSize( myStageCellSize );
		stage.setColor( sf::Color(myPanelColor), sf::Color(mDrawingInfo.cellOutlineColor) );
		stage.setOutline( myPanelOutlineThickness, sf::Color(myPanelOutlineColor) );
		if ( false == playView.loadCountdownSprite(mDrawingInfo.countdownSpritePath) )
		{
			gService()->console().printScriptError( ExceptionType::FILE_NOT_FOUND,
													"Path:CountdownSprite", scriptPath );
		}
		playView.setCountdownSpriteDimension( myPanelPosition, myStageCellSize, mDrawingInfo.countdownSpriteClipSize );
		::model::Tetrimino& tetrimino =	playView.currentTetrimino();
		tetrimino.setOrigin( myPanelPosition );
		tetrimino.setSize( myStageCellSize );
		::vfx::Combo& vfxCombo = playView.vfxCombo();
		vfxCombo.setOrigin( myPanelPosition, myStageCellSize, vfxComboClipSize );
		if ( false == vfxCombo.loadResources(vfxComboPath) )
		{
			gService()->console().printFailure( FailureLevel::WARNING,
												 "File Not Found: "+vfxComboPath );
		}
		::ui::NextTetriminoPanel& nextTetPanel = playView.nextTetriminoPanel();
		nextTetPanel.setDimension( nextTetriminoPanelPosition, nextTetriminoPanelCellSize );
		mDrawingInfo.nextTetriminoPanelBound = nextTetPanel.globalBounds();
		// Making the bound enough to contain a mouse cursor.
		mDrawingInfo.nextTetriminoPanelBound.width += 5.f;
		mDrawingInfo.nextTetriminoPanelBound.height += 5.f;
		nextTetPanel.setColor( sf::Color(nextTetriminoPanelColor), sf::Color(mDrawingInfo.cellOutlineColor) );
		nextTetPanel.setOutline( nextTetriminoPanelOutlineThickness, sf::Color(nextTetriminoPanelOutlineColor) );
	}
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
	
	if ( false == mFont.loadFromFile(fontPath) )
	{
		gService()->console().printFailure( FailureLevel::WARNING,
											 "File Not Found: "+fontPath );
	}
	mLabelForNickname.setFont( mFont );
	if ( true == mAsHost )
	{
		mTextLabelForStartingGuide.setString( "Go!" );
	}
	else
	{
		mTextLabelForStartingGuide.setString( "Be\nPrepared!" );
	}
	mTextLabelForStartingGuide.setFont( mFont );
	mTextLabelForStartingGuide.setCharacterSize( startingGuideFontSize );
	const sf::FloatRect bound( mTextLabelForStartingGuide.getLocalBounds() );
	const sf::Vector2f center( bound.width*0.5f, bound.height*0.5f );
	mTextLabelForStartingGuide.setOrigin( center );
	mTextLabelForStartingGuide.setPosition( startingGuidePosition );
	mTextLabelForStartingGuide.setFillColor( sf::Color(startingGuideFontColor) );

	::ui::PlayView::LoadResources( );
}

::scene::online::ID scene::online::InRoom::update( std::vector<sf::Event>& eventQueue,
												  ::scene::online::Online& net,
												  sf::RenderWindow& window )
{
	::scene::online::ID nextSceneID = ::scene::online::ID::AS_IS;
	const HashedKey myNicknameHashed = net.myNicknameHashed();
	if ( true == net.hasReceived() )
	{
		mIsReceiving = false;

		if ( const std::optional<std::string> hasLeftOrKicked( net.getByTag(TAGGED_REQ_LEAVE_ROOM,
																			 Online::Option::RETURN_TAG_ATTACHED,
																			 0) );
			std::nullopt != hasLeftOrKicked )
		{
			nextSceneID = ::scene::online::ID::IN_LOBBY;
			return nextSceneID;
		}

		if ( const std::optional<std::string> userList( net.getByTag(TAGGED_NOTI_UPDATE_USER_LIST,
																	 Online::Option::DEFAULT,
																	 -1) );
			std::nullopt != userList )
		{
			const std::string& _userList( userList.value() );
			const uint16_t userListSize = (uint16_t)_userList.size();
			const char* const ptr = _userList.data();
			std::unordered_map<HashedKey, std::string> users;
			uint16_t curPos = 0;
			while ( userListSize != curPos )
			{
				const uint8_t curSize = ptr[curPos];
				++curPos;
				const std::string otherNickname( _userList.substr(curPos, curSize) );
				const HashedKey otherNicknameHashed = ::util::hash::Digest2( otherNickname.data() );
				users.emplace( otherNicknameHashed, otherNickname );
				curPos += curSize;
			}
			for ( auto it = mParticipants.begin(); mParticipants.end() != it; )
			{
				if ( users.end() == users.find(it->first) )
				{
					if ( it->first == myNicknameHashed )
					{
						nextSceneID = ::scene::online::ID::IN_LOBBY;
						return nextSceneID;
					}
					for ( HashedKey& nicknameHashed : mOtherPlayerSlots )
					{
						if ( it->first == nicknameHashed )
						{
							nicknameHashed = NULL_EMPTY_SLOT;
							break;
						}
					}
					it = mParticipants.erase(it);
				}
				else
				{
					users.erase( it->first );
					++it;
				}
			}
			for ( const auto& pair : users )
			{
				mParticipants.emplace( pair.first, Participant(pair.second, ::ui::PlayView(false)) );
				uint8_t slotIdx = 0;
				while ( ROOM_CAPACITY-1 != slotIdx )
				{
					if ( NULL_EMPTY_SLOT == mOtherPlayerSlots[slotIdx] )
					{
						mOtherPlayerSlots[slotIdx] = pair.first;
						break;
					}
					++slotIdx;
				}
				ASSERT_TRUE( ROOM_CAPACITY-1 != slotIdx );
				::ui::PlayView& playView = mParticipants.find(pair.first)->second.playView;
				::model::Stage& stage =	playView.stage();
				const sf::Vector2f pos( mDrawingInfo.position-mDrawingInfo.positionDifferences[slotIdx] );
				stage.setPosition( pos );
				stage.setSize( mDrawingInfo.cellSize );
				const sf::Color cellOutlineColor( 0x000000'7f );
				stage.setColor( sf::Color(mDrawingInfo.panelColor_on), sf::Color(cellOutlineColor) );
				stage.setOutline( mDrawingInfo.outlineThickness_on, sf::Color(mDrawingInfo.outlineColor_on) );
				::model::Tetrimino& curTet = playView.currentTetrimino();
				curTet.setOrigin( pos );
				curTet.setSize( mDrawingInfo.cellSize );
				if ( false == playView.loadCountdownSprite(mDrawingInfo.countdownSpritePath) )
				{
					gService()->console().printFailure( FailureLevel::WARNING,
														"Can't find a countdown sprite." );
				}
				playView.setCountdownSpriteDimension( pos,
													mDrawingInfo.cellSize,
													 mDrawingInfo.countdownSpriteClipSize );
			}
		}

		if ( const std::optional<std::string> hostChanged( net.getByTag(TAGGED_NOTI_HOST_CHANGED,
																		 Online::Option::DEFAULT,
																		 sizeof(HashedKey)) );
			std::nullopt != hostChanged	)
		{
			const std::string& _newHostNicknameHashed = hostChanged.value();
			const HashedKey newHostNicknameHashed = ::ntohl(*(HashedKey*)_newHostNicknameHashed.data());
			if ( newHostNicknameHashed == net.myNicknameHashed() )
			{
				mAsHost = true;
			}
		}

		if ( const std::optional<std::string> gettingReady( net.getByTag(TAGGED_REQ_GET_READY,
																		Online::Option::RETURN_TAG_ATTACHED,
																		0) );
			std::nullopt != gettingReady )
		{
			for ( auto& pair : mParticipants )
			{
				pair.second.playView.getReady( );
			}
			mIsStartingGuideVisible_ = false;
			gService()->sound().stopBGM( );
		}

		if ( const std::optional<std::string> gamesOver( net.getByTag(TAG_GAMES_OVER,
																	Online::Option::DEFAULT,
																	-1) );
			std::nullopt != gamesOver )
		{
			const std::string& _gamesOver = gamesOver.value();
			const uint16_t totalSize = (uint16_t)_gamesOver.size();
			const char* ptr = _gamesOver.data();
			uint16_t curPos = 0;
			while ( totalSize != curPos )
			{
				const HashedKey nicknameHashed = ::ntohl(*(HashedKey*)&ptr[curPos]);
				curPos += sizeof(HashedKey);
				if ( const auto it = mParticipants.find(nicknameHashed);
					mParticipants.end() != it )
				{
					it->second.playView.gameOver();
				}
			}
		}

		if ( const std::optional<std::string> newCurrentTetriminos( net.getByTag(TAG_NEW_CURRENT_TETRIMINOS,
																				Online::Option::DEFAULT,
																				-1) );
			std::nullopt != newCurrentTetriminos )
		{
			const std::string& newCurTetTypes = newCurrentTetriminos.value();
			const char* ptr = newCurTetTypes.data();
			const uint16_t totalSize = (uint16_t)newCurTetTypes.size();
			uint16_t curPos = 0;
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
			}
		}

		if ( const std::optional<std::string> currentTetriminosMove( net.getByTag(TAG_CURRENT_TETRIMINOS_MOVE,
																				Online::Option::DEFAULT,
																				-1) );
			std::nullopt != currentTetriminosMove )
		{
			const std::string& curTetsMove = currentTetriminosMove.value();
			const char* ptr = curTetsMove.data();
			const uint16_t totalSize = (uint16_t)curTetsMove.size();
			uint16_t curPos = 0;
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
			}
		}

		if ( const std::optional<std::string> stages( net.getByTag(TAG_STAGES,
																	Online::Option::DEFAULT,
																	-1) );
			std::nullopt != stages )
		{
			const std::string& _stages = stages.value();
			const char* ptr = _stages.data();
			const uint16_t totalSize = (uint16_t)_stages.size();
			uint16_t curPos = 0;
			while ( totalSize != curPos )
			{
				const HashedKey nicknameHashed = ::ntohl(*(HashedKey*)&ptr[curPos]);
				curPos += sizeof(HashedKey);
				const ::model::stage::Grid* const grid = (::model::stage::Grid*)&ptr[curPos];
				curPos += sizeof(::model::stage::Grid);
				if ( const auto it = mParticipants.find(nicknameHashed);
					mParticipants.end() != it )
				{
					it->second.playView.updateStage( *grid );
				}
			}
		}

		if ( const std::optional<std::string> numsOfLinesCleared( net.getByTag(TAG_NUMS_OF_LINES_CLEARED,
																				Online::Option::DEFAULT,
																				-1) );
			std::nullopt != numsOfLinesCleared )
		{
			const std::string& _numsOfLinesCleared = numsOfLinesCleared.value();
			const char* ptr = _numsOfLinesCleared.data();
			const uint16_t totalSize = (uint16_t)_numsOfLinesCleared.size();
			uint16_t curPos = 0;
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
			}
		}

		if ( const std::optional<std::string> allOver( net.getByTag(TAG_ALL_OVER,
																	Online::Option::RETURN_TAG_ATTACHED,
																	0) );
					std::nullopt != allOver )
		{
			mAlarms[(int)AlarmIndex::ALL_OVER_FREEZE] = Clock::now();
			if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::GAME_OVER]) )
			{
				gService()->console().printFailure(FailureLevel::WARNING,
												   "File Not Found: "+mSoundPaths[(int)SoundIndex::GAME_OVER] );
			}
		}
	}

	for ( auto& pair : mParticipants )
	{
		pair.second.playView.update( eventQueue, net );
	}

	if ( false == mIsReceiving )
	{
		net.receive( );
		mIsReceiving = true;
	}

	if ( true == alarmAfterAndReset(ALL_OVER_FREEZE_MS, AlarmIndex::ALL_OVER_FREEZE) )
	{
		// Resetting remnants once and for all.
		for ( auto& pair : mParticipants )
		{
			::ui::PlayView& playView = pair.second.playView;
			playView.stage().clear();
		}
		mIsStartingGuideVisible_ = true;
	}

	if ( nullptr != mOverlappedScene )
	{
		const ::scene::inPlay::ID nextScene = mOverlappedScene->update(eventQueue);
		switch ( nextScene )
		{
			case ::scene::inPlay::ID::EXIT:
				leaveRoom( );
				break;
			case ::scene::inPlay::ID::UNDO:
				mOverlappedScene.reset( );
				break;
			case ::scene::inPlay::ID::AS_IS:
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

	for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
	{
		if ( sf::Event::EventType::MouseButtonPressed == it->type &&
			sf::Mouse::Button::Left == it->mouseButton.button &&
			true == mIsMouseOverStartButton_ &&
			true == mIsStartingGuideVisible_ )
		{
			mIsStartButtonPressed_ = true;
			it = eventQueue.erase(it);
		}
		else if ( sf::Event::EventType::MouseButtonReleased == it->type &&
			sf::Mouse::Button::Left == it->mouseButton.button &&
			true == mIsMouseOverStartButton_ &&
			true == mIsStartingGuideVisible_ )
		{
			startGame( );
			if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::ON_SELECTION]) )
			{
				gService()->console().printFailure(FailureLevel::WARNING,
												   "File Not Found: "+mSoundPaths[(int)SoundIndex::ON_SELECTION] );
			}
			mIsStartButtonPressed_ = false;
			it = eventQueue.erase(it);
		}
		else if ( sf::Event::EventType::KeyPressed == it->type &&
					sf::Keyboard::Escape == it->key.code )
		{
			mOverlappedScene = std::make_unique<::scene::inPlay::Assertion>(window);
			it = eventQueue.erase(it);
		}
		else
		{
			++it;
		}
	}

	return nextSceneID;
}

void scene::online::InRoom::draw( sf::RenderWindow& window )
{
	window.draw( mBackground );
	{
		const auto it = mParticipants.find(mMyNicknameHashed);
		ASSERT_TRUE( mParticipants.end() != it );
		if ( true == mAsHost )
		{
			::ui::NextTetriminoPanel& nextTetPanel = it->second.playView.nextTetriminoPanel();
			const sf::Vector2f mousePos( sf::Mouse::getPosition()-window.getPosition() );
			if ( true == mDrawingInfo.nextTetriminoPanelBound.contains(mousePos) )
			{
				if ( false == mIsMouseOverStartButton_ )
				{
					mTextLabelForStartingGuide.setString( "!" );
					const sf::FloatRect bound( mTextLabelForStartingGuide.getLocalBounds() );
					const sf::Vector2f center( bound.width*0.5f, bound.height*0.5f );
					mTextLabelForStartingGuide.setOrigin( center );
				}
				mIsMouseOverStartButton_ = true;
				if ( 0 == mFrameCountCoolToRotateStartButton )
				{
					nextTetPanel.rotate( mDrawingInfo.angularVelocity );
					float degree = nextTetPanel.rotation();
					while ( mDrawingInfo.arcLength <= degree )
					{
						degree -= mDrawingInfo.arcLength;
					}
					if ( degree < mDrawingInfo.angularVelocity/2.f )
					{
						mFrameCountCoolToRotateStartButton = 1;
					}
				}
				else if ( mDrawingInfo.framesRotationInterval < ++mFrameCountCoolToRotateStartButton )
				{
					mFrameCountCoolToRotateStartButton = 0;
				}
				if ( true == mIsStartButtonPressed_ )
				{
					nextTetPanel.scale( mDrawingInfo.scaleFactor );
					mIsStartButtonPressed_ = false;
				}
			}
			else
			{
				mTextLabelForStartingGuide.setString( "Start" );
				const sf::FloatRect bound( mTextLabelForStartingGuide.getLocalBounds() );
				const sf::Vector2f center( bound.width*0.5f, bound.height*0.5f );
				mTextLabelForStartingGuide.setOrigin( center );
				nextTetPanel.resetScale( );
				nextTetPanel.resetRotation( );
				mFrameCountCoolToRotateStartButton = 0;
				mIsMouseOverStartButton_ = false;
			}
		}
		it->second.playView.draw( window );
		if ( true == mIsStartingGuideVisible_ )
		{
			window.draw( mTextLabelForStartingGuide );
		}
		const sf::Vector2f margin( 10.f, 0.f );
		mLabelForNickname.setString( mMyNickname );
		mLabelForNickname.setCharacterSize( mDrawingInfo.myNicknameFontSize );
		mLabelForNickname.setPosition( it->second.playView.stage().position() + margin );
		mLabelForNickname.setFillColor( sf::Color(mDrawingInfo.myNicknameColor) );
		window.draw( mLabelForNickname );
	}
	
	for ( uint8_t i = 0; ROOM_CAPACITY-1 != i; ++i )
	{
		const sf::Vector2f pos( mDrawingInfo.position-mDrawingInfo.positionDifferences[i] );
		const sf::Vector2f margin( 10.f, 0.f );
		if ( NULL_EMPTY_SLOT == mOtherPlayerSlots[i] )
		{
			mOtherPlayerSlotBackground.setPosition( pos );
			window.draw( mOtherPlayerSlotBackground );
		}
		else
		{
			const auto it = mParticipants.find(mOtherPlayerSlots[i]);
			ASSERT_TRUE( mParticipants.end() != it );
			it->second.playView.draw( window );
			mLabelForNickname.setString( it->second.nickname );
			mLabelForNickname.setCharacterSize( mDrawingInfo.otherPlayerNicknameFontSize );
			mLabelForNickname.setFillColor( sf::Color(mDrawingInfo.otherPlayerNicknameFontColor) );
			mLabelForNickname.setPosition( pos + margin );
			window.draw( mLabelForNickname );
		}
	}

	if ( nullptr != mOverlappedScene )
	{
		mOverlappedScene->draw( window );
	}
}

void scene::online::InRoom::startGame( ) const
{
	std::string request( TAGGED_REQ_START_GAME );
	mNet.send( request.data(), (int)request.size() );
}

void scene::online::InRoom::_startGame( const std::string_view& arg )
{
	startGame( );
}

void scene::online::InRoom::leaveRoom( ) const
{
	std::string request( TAGGED_REQ_LEAVE_ROOM );
	mNet.send( request.data(), (int)request.size() );
}

void scene::online::InRoom::_leaveRoom( const std::string_view& )
{
	leaveRoom( );
}
