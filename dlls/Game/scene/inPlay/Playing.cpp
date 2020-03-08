#include "../../pch.h"
#include "Playing.h"
#include "../../ServiceLocatorMirror.h"
#include "GameOver.h"
#include "Assertion.h"

const float TEMPO_DIFF_RATIO = 0.02f;
// Disposition per frame when a tetrimino falls down, or hard drops.
const uint8_t HARD_DROP_SPEED = 3;
const uint16_t LINE_CLEAR_CHK_INTERVAL_MS = 100;
const uint16_t COOL_TIME_TO_NEXT_LEVEL_OR_OVER_MS = 1000;
const uint16_t COOL_TIME_ALL_LEVELS_CLEARED_MS = 1000;
const uint16_t ANIMATION_SPEED_MS = 34;

bool ::scene::inPlay::Playing::IsInstantiated = false;

::scene::inPlay::Playing::Playing( const sf::RenderWindow& window,
								   sf::Drawable& shapeOrSprite,
								   const std::unique_ptr<::scene::inPlay::IScene>& overlappedScene )
	: mNumOfLinesRecentlyCleared( 0 ), mCurrentLevel( 1 ),
	mNumOfLinesRemainingToLevelClear( 7 ),
	mFrameCountSoftDropInterval( 0 ), mFrameCountLineClearInterval_( 0 ),
	mFrameCountVfxDuration( 0 ),
	mFrameCountCool( 0 ),
	mStateAfterCooling( StateAfterCooling::NONE ),
	mAnimationDamperForScore1( 0 ), mAnimationDamperScore10( 0 ),
	mTempo( 0.75f ),
	mBackgroundRect_( (sf::RectangleShape&)shapeOrSprite ),
	mOverlappedScene_( overlappedScene )
{
	ASSERT_TRUE( false == IsInstantiated );

	mSpriteForScore.setTexture( mTextureForScore );
	loadResources( window );
	gService()->sound().stopBGM( );
	mCurrentTetrimino = ::model::Tetrimino::Spawn();
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );

	IsInstantiated = true;
}

::scene::inPlay::Playing::~Playing()
{
	IsInstantiated = false;
}

void ::scene::inPlay::Playing::loadResources( const sf::RenderWindow& )
{
	uint32_t backgroundColor = 0x29cdb5'fa; // CYAN
	mDrawingInfo.blackOutColor = 0x808080ff; // GRAY
	sf::Vector2f stagePanelPosition( 130.0, 0.0 );
	float stageCellSize = 30.0;
	uint32_t stagePanelColor = 0x3f3f3f'ff;
	float stagePanelOutlineThickness = 11.0;
	uint32_t stagePanelOutlineColor = 0x3f3f3f'7f;
	uint32_t stageCellOutlineColor = 0x000000'7f;
	std::string vfxComboSpritePath( "Vfxs/Combo.png" );
	sf::Vector2i vfxComboClipSize( 256, 256 );
	sf::Vector2f nextTetPanelPosition( 525.0, 70.0 );
	float nextTetPanelCellSize = 30.0;
	uint32_t nextTetPanelColor = 0x000000'ff;
	float nextTetPanelOutlineThickness = 5.0;
	uint32_t nextTetPanelOutlineColor = 0x000000'7f;
	uint32_t nextTetPanelCellOutlineColor = 0x000000'7f;
	mSoundPaths[(int)SoundIndex::TETRIMINO_LOCK] = "Sounds/tetriminoLocked.wav";
	mSoundPaths[(int)SoundIndex::LINE_CLEAR] = "Sounds/lineCleared.wav";
	mSoundPaths[(int)SoundIndex::LEVEL_CLEAR] = "Sounds/levelCleared.wav";
	std::string scoreSpritePath( "Images/Score.png" );
	mDrawingInfo.scoreSpriteClipSize.x = 128;
	mDrawingInfo.scoreSpriteClipSize.y = 128;
	mDrawingInfo.scorePosition.x = 450.f;
	mDrawingInfo.scorePosition.y = 350.f;
	mDrawingInfo.gapBetweenScoreLetter = 60.f;
	mDrawingInfo.animationSpeed = 1.f;

	lua_State* lua = luaL_newstate();
	const std::string scriptPath( "Scripts/Playing.lua" );
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

		varName = "BlackOutColor";
		lua_getglobal( lua, varName.data() );
		type = lua_type(lua, TOP_IDX);
		if ( LUA_TNUMBER == type )
		{
			mDrawingInfo.blackOutColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
		lua_getglobal( lua, tableName.data( ) );
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
				stagePanelPosition.x = (float)lua_tonumber(lua, TOP_IDX);
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
				stagePanelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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
				stageCellSize = (float)lua_tonumber(lua, TOP_IDX);
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
				stagePanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
				stagePanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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
				stagePanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "cellOutlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				stageCellOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
		if ( false == lua_istable( lua, TOP_IDX ) )
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
				vfxComboSpritePath = lua_tostring(lua, TOP_IDX);
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
		if ( false == lua_istable( lua, TOP_IDX ) )
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
				nextTetPanelPosition.x = (float)lua_tonumber(lua, TOP_IDX);
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
				nextTetPanelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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
				nextTetPanelCellSize = (float)lua_tonumber(lua, TOP_IDX);
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
				nextTetPanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
				nextTetPanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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
				nextTetPanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "cellOutlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelCellOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
			std::string innerTableName( "tetriminoLocked" );
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
					mSoundPaths[(int)SoundIndex::TETRIMINO_LOCK] = lua_tostring(lua, TOP_IDX);
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

			innerTableName = "lineCleared";
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
					mSoundPaths[(int)SoundIndex::LINE_CLEAR] = lua_tostring(lua, TOP_IDX);
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

			innerTableName = "levelCleared";
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
					mSoundPaths[(int)SoundIndex::LEVEL_CLEAR] = lua_tostring(lua, TOP_IDX);
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
		
		tableName = "Missions";
		lua_getglobal( lua, tableName.data() );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			gService()->console().printScriptError( ExceptionType::TYPE_CHECK,
												   tableName, scriptPath );
		}
		else
		{
			// NOTE: Mutex isn't being used now.
			mMissions.clear( );
			uint8_t lvIndex = 0;
			while ( true )
			{
				lua_pushinteger( lua, ++lvIndex );
				lua_gettable( lua, 1 );
				if ( false == lua_istable(lua, TOP_IDX) )
				{
					lua_pop( lua, 1 );
					break;
				}
				else
				{
					uint8_t innerIdx = 1;
					Mission mission;
					lua_pushinteger( lua, innerIdx );
					lua_gettable( lua, 2 );
					type = lua_type(lua, TOP_IDX);
					if ( LUA_TNUMBER == type )
					{
						mission.numOfLinesToClear = (uint8_t)lua_tointeger(lua, TOP_IDX);
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

					++innerIdx;
					lua_pushinteger( lua, innerIdx );
					lua_gettable( lua, 2 );
					type = lua_type(lua, TOP_IDX);
					if ( LUA_TNUMBER == type )
					{
						mission.tempoOnStart = (float)lua_tonumber(lua, TOP_IDX);
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

					mMissions.emplace_back( mission );
				}
				lua_pop( lua, 1 );
			}
		}
		lua_pop( lua, 1 );

		tableName = "Score";
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
				scoreSpritePath = lua_tostring(lua, TOP_IDX);
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
				mDrawingInfo.scoreSpriteClipSize.x = (int)lua_tointeger(lua, TOP_IDX);
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
				mDrawingInfo.scoreSpriteClipSize.y = (int)lua_tointeger(lua, TOP_IDX);
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
				mDrawingInfo.scorePosition.x = (float)lua_tonumber(lua, TOP_IDX);
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
				mDrawingInfo.scorePosition.y = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "gap";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.gapBetweenScoreLetter = (float)lua_tonumber(lua, TOP_IDX);
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

			field = "animationSpeed";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type(lua, TOP_IDX);
			if ( LUA_TNUMBER == type )
			{
				mDrawingInfo.animationSpeed = (float)lua_tonumber(lua, TOP_IDX);
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
	lua_close( lua );

	if ( false == mVfxCombo.loadResources( vfxComboSpritePath ) )
	{
		gService()->console().printFailure( FailureLevel::WARNING,
												"File Not Found: "+vfxComboSpritePath );
	}
	mBackgroundRect_.setFillColor( sf::Color(backgroundColor) );
	mCurrentTetrimino.setOrigin( stagePanelPosition );
	mCurrentTetrimino.setSize( stageCellSize );
	mStage.setPosition( stagePanelPosition );
	mStage.setSize( stageCellSize );
	mStage.setColor( sf::Color(stagePanelColor), sf::Color(stageCellOutlineColor) );
	mStage.setOutline( stagePanelOutlineThickness, sf::Color(stagePanelOutlineColor) );
	mVfxCombo.setOrigin( stagePanelPosition, stageCellSize, vfxComboClipSize );
	mNextTetriminoPanel.setDimension( nextTetPanelPosition, nextTetPanelCellSize );
	mNextTetriminoPanel.setColor( sf::Color(nextTetPanelColor), sf::Color(nextTetPanelCellOutlineColor) );
	mNextTetriminoPanel.setOutline( nextTetPanelOutlineThickness, sf::Color(nextTetPanelOutlineColor) );
	if ( false == mNextTetriminos.empty() )
	{
		mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
	}
	mDrawingInfo.cellSize = stageCellSize;
	ASSERT_TRUE( 0 < mMissions.size() );
	mNumOfLinesRemainingToLevelClear = mMissions[mCurrentLevel-1].numOfLinesToClear;
	mTempo = mMissions[mCurrentLevel-1].tempoOnStart;
	if ( false == mTextureForScore.loadFromFile(scoreSpritePath) )
	{
		gService()->console().printFailure( FailureLevel::WARNING,
										   "File Not Found: "+scoreSpritePath );
	}

	::model::Tetrimino::LoadResources( );
}

::scene::inPlay::ID scene::inPlay::Playing::update( std::vector<sf::Event>& eventQueue )
{
	::scene::inPlay::ID retVal = ::scene::inPlay::ID::AS_IS;
	uint16_t fps = 60;
	{
		auto& vault = gService()->vault();
		const auto it = vault.find(HK_FORE_FPS);
		ASSERT_TRUE( vault.end() != it );
		fps = (uint16_t)it->second;
		switch( mStateAfterCooling )
		{
			case StateAfterCooling::GAME_OVER:
				if ( fps*COOL_TIME_TO_NEXT_LEVEL_OR_OVER_MS/1000 < mFrameCountCool )
				{
					retVal = ::scene::inPlay::ID::GAME_OVER;
					return retVal;
				}
				else if ( 0 != mFrameCountCool )
				{
					return retVal;
				}
				break;
			case StateAfterCooling::NEXT_LEVEL:
				if ( fps*COOL_TIME_TO_NEXT_LEVEL_OR_OVER_MS/1000 < mFrameCountCool )
				{
					mStateAfterCooling = StateAfterCooling::NONE;
					mFrameCountCool = 0;
					++mCurrentLevel;
					ASSERT_TRUE( mCurrentLevel <= mMissions.size() );
					mNumOfLinesRemainingToLevelClear = mMissions[mCurrentLevel-1].numOfLinesToClear;
					mTempo = mMissions[mCurrentLevel-1].tempoOnStart;
					mStage.clear( );
					reloadTetrimino( );
					return retVal;
				}
				else if ( 0 != mFrameCountCool )
				{
					return retVal;
				}
				break;
			case StateAfterCooling::ALL_CLEAR:
				if ( fps*COOL_TIME_ALL_LEVELS_CLEARED_MS/1000 < mFrameCountCool )
				{
					mStateAfterCooling = StateAfterCooling::NONE;
					mFrameCountCool = 0;
					return retVal;
				}
				else if ( 0 != mFrameCountCool )
				{
					return retVal;
				}
				break;
			default:
				break;
		}
	}

	bool hasTetriminoLanded = false;
	if ( true == mCurrentTetrimino.isHardDropping( ) )
	{
		for ( uint8_t i = 0; i != HARD_DROP_SPEED; ++i )
		{
			hasTetriminoLanded = mCurrentTetrimino.moveDown(mStage.cgrid());
			if ( true == hasTetriminoLanded )
			{
				mCurrentTetrimino.hardDrop( false );
				break;
			}
		}
		if ( false == hasTetriminoLanded )
		{
			return ::scene::inPlay::ID::AS_IS;
		}
	}
	else
	{
		for ( auto it = eventQueue.cbegin(); eventQueue.cend() != it; )
		{
			if ( sf::Event::KeyPressed == it->type )
			{
				switch ( it->key.code )
				{
					case sf::Keyboard::Space:
						mCurrentTetrimino.hardDrop( );
						mFrameCountSoftDropInterval = 0;
						it = eventQueue.erase(it);
						break;
					case sf::Keyboard::Down:
						hasTetriminoLanded = mCurrentTetrimino.moveDown( mStage.cgrid( ) );
						mFrameCountSoftDropInterval = 0;
						it = eventQueue.erase(it);
						break;
					case sf::Keyboard::Left:
						mCurrentTetrimino.tryMoveLeft( mStage.cgrid( ) );
						it = eventQueue.erase(it);
						break;
					case sf::Keyboard::Right:
						mCurrentTetrimino.tryMoveRight( mStage.cgrid( ) );
						it = eventQueue.erase(it);
						break;
					case sf::Keyboard::LShift:
						[[ fallthrough ]];
					case sf::Keyboard::Up:
						mCurrentTetrimino.tryRotate( mStage.cgrid( ) );
						it = eventQueue.erase(it);
						break;
					case sf::Keyboard::Escape:
						if ( nullptr == mOverlappedScene_ ||
							typeid(*mOverlappedScene_) != typeid(::scene::inPlay::Assertion) )
						{
							retVal = ::scene::inPlay::ID::ASSERTION;
							it = eventQueue.erase(it);
						}
						else
						{
							++it;
						}
						break;
					default:
						++it;
						break;
				}
			}
			else
			{
				++it;
			}
		}
	}
	
	if ( (uint16_t)fps*mTempo < mFrameCountSoftDropInterval )
	{
		hasTetriminoLanded = mCurrentTetrimino.moveDown(mStage.cgrid());
		mFrameCountSoftDropInterval = 0;
	}

	if ( true == hasTetriminoLanded )
	{
		mCurrentTetrimino.land( mStage.grid() );
		if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::TETRIMINO_LOCK]) )
		{
			gService()->console().printFailure(FailureLevel::WARNING,
											   "File Not Found: "+mSoundPaths[(int)SoundIndex::TETRIMINO_LOCK] );
		}
		reloadTetrimino( );
	}

	// Check if a row or more have to be cleared,
	// NOTE: It's better to check that every several frames than every frame.
	if ( (uint16_t)fps*LINE_CLEAR_CHK_INTERVAL_MS/1000 < mFrameCountLineClearInterval_ )
	{
		mFrameCountLineClearInterval_ = 0;
		const uint8_t numOfLinesCleared = mStage.tryClearRow();
		if ( 0 != numOfLinesCleared )
		{
			mNumOfLinesRecentlyCleared = numOfLinesCleared;
			if ( 0 < mNumOfLinesRemainingToLevelClear &&
				mNumOfLinesRemainingToLevelClear <= numOfLinesCleared )
			{
				mNumOfLinesRemainingToLevelClear -= numOfLinesCleared;
				if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::LEVEL_CLEAR]) )
				{
					gService()->console().printFailure(FailureLevel::WARNING,
													   "File Not Found: "+mSoundPaths[(int)SoundIndex::LEVEL_CLEAR] );
				}
				if ( mMissions.size() == mCurrentLevel )
				{
					mStateAfterCooling = StateAfterCooling::ALL_CLEAR;
					retVal = ::scene::inPlay::ID::ALL_CLEAR;
					return retVal;
				}
				else
				{
					mStateAfterCooling = StateAfterCooling::NEXT_LEVEL;
				}
			}
			else
			{
				if ( 0 < mNumOfLinesRemainingToLevelClear )
				{
					const uint8_t digit10 = mNumOfLinesRemainingToLevelClear/10;
					mNumOfLinesRemainingToLevelClear -= numOfLinesCleared;
					if ( digit10 != mNumOfLinesRemainingToLevelClear/10 )
					{
						mAnimationDamperScore10 = mDrawingInfo.scoreSpriteClipSize.y;
					}
					mAnimationDamperForScore1 = numOfLinesCleared;
					if ( mNumOfLinesRemainingToLevelClear < 0 )
					{
						mAnimationDamperForScore1 += mNumOfLinesRemainingToLevelClear;
					}
					mAnimationDamperForScore1 *= mDrawingInfo.scoreSpriteClipSize.y;
				}
				mTempo -= TEMPO_DIFF_RATIO;
				// Triggering.
				mFrameCountVfxDuration = 1;
				if ( false == gService()->sound().playSFX(mSoundPaths[(int)SoundIndex::LINE_CLEAR]) )
				{
					gService()->console().printFailure(FailureLevel::WARNING,
													   "File Not Found: "+mSoundPaths[(int)SoundIndex::LINE_CLEAR] );
				}
			}
		}
		else if ( true == mStage.isOver() )
		{
			mStage.blackout( sf::Color(mDrawingInfo.blackOutColor) );
			mStateAfterCooling = StateAfterCooling::GAME_OVER;
		}
	}
	
	return retVal;
}

void ::scene::inPlay::Playing::draw( sf::RenderWindow& window )
{
	window.draw( mBackgroundRect_ );
	mStage.draw( window );
	if ( mStateAfterCooling == StateAfterCooling::NEXT_LEVEL ||
		mStateAfterCooling == StateAfterCooling::GAME_OVER )
	{
		++mFrameCountCool;
	}
	else
	{
		mCurrentTetrimino.draw( window );
		if ( mStateAfterCooling == StateAfterCooling::ALL_CLEAR )
		{
			++mFrameCountCool;
		}
	}
	mNextTetriminoPanel.draw( window );
	{
		uint8_t score = mNumOfLinesRemainingToLevelClear;
		if ( score < 0 )
		{
			score = 0;
		}
		mSpriteForScore.setTextureRect(	sf::IntRect(0, (score/10)*mDrawingInfo.scoreSpriteClipSize.y+mAnimationDamperScore10,
													mDrawingInfo.scoreSpriteClipSize.x,
													mDrawingInfo.scoreSpriteClipSize.y) );
		mSpriteForScore.setPosition( mDrawingInfo.scorePosition );
		window.draw( mSpriteForScore );
		mSpriteForScore.setTextureRect(	sf::IntRect(0, (score%10)*mDrawingInfo.scoreSpriteClipSize.y+mAnimationDamperForScore1,
													mDrawingInfo.scoreSpriteClipSize.x,
													mDrawingInfo.scoreSpriteClipSize.y) );
		mSpriteForScore.setPosition( mDrawingInfo.scorePosition +
									sf::Vector2f(mDrawingInfo.gapBetweenScoreLetter, 0.f) );
		window.draw( mSpriteForScore );
		mSpriteForScore.setTextureRect(	sf::IntRect(0, mDrawingInfo.scoreSpriteClipSize.y*14,
													mDrawingInfo.scoreSpriteClipSize.x,
													mDrawingInfo.scoreSpriteClipSize.y) );
		mSpriteForScore.setPosition( mDrawingInfo.scorePosition +
									sf::Vector2f(mDrawingInfo.gapBetweenScoreLetter+30.f, 75.f) );
		window.draw( mSpriteForScore );
	}
	if ( 0 != mFrameCountVfxDuration )
	{
		mVfxCombo.draw( window, mNumOfLinesRecentlyCleared );
		++mFrameCountVfxDuration;
	}
	
	++mFrameCountLineClearInterval_;
	++mFrameCountSoftDropInterval;
	auto& vault = gService()->vault();
	const auto it = vault.find(HK_FORE_FPS);
	ASSERT_TRUE( vault.end() != it );
	const uint16_t fps = (uint16_t)it->second;
	if ( fps <= mFrameCountVfxDuration )
	{
		mFrameCountVfxDuration = 0;
	}
	{
		const int16_t delta = (int16_t)(2*mDrawingInfo.animationSpeed);
		if ( 0 != mAnimationDamperForScore1 )
		{
			mAnimationDamperForScore1 -= delta;
		}
		if ( 0 != mAnimationDamperScore10 )
		{
			mAnimationDamperScore10 -= delta;
		}
	}
}

void scene::inPlay::Playing::reloadTetrimino( )
{
	mCurrentTetrimino = mNextTetriminos.front();
	mCurrentTetrimino.setOrigin( mStage.position() );
	mCurrentTetrimino.setSize( mDrawingInfo.cellSize );
	mNextTetriminos.pop( );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
	mFrameCountSoftDropInterval = 0;
}
