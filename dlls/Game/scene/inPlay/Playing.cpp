#include "../../pch.h"
#include "Playing.h"
#include "../../ServiceLocatorMirror.h"
#include "GameOver.h"
#include "Assertion.h"
#include "../VaultKeyList.h"

const float TEMPO_DIFF_RATIO = 0.02f;
const uint8_t FALLING_DOWN_SPEED = 3;
const uint32_t LINE_CLEAR_CHK_INTERVAL_MS = 100;

::scene::inPlay::Playing::Playing( sf::RenderWindow& window,
								   sf::Drawable& shapeOrSprite,
								   const std::unique_ptr<::scene::inPlay::IScene>& overlappedScene )
	: mNumOfLinesCleared( 0 ),
	mFrameCount_fallDown( 0 ), mFrameCount_clearingInterval_( 0 ),
	mFrameCount_clearingVfx_( 0 ), mFrameCount_gameOver( 0 ),
	mTempo( 0.75f ),
	mWindow_( window ), mBackgroundRect_( (sf::RectangleShape&)shapeOrSprite ),
	mOverlappedScene_( overlappedScene ),
	mNextTetriminoPanel( window ),
	mVfxCombo( window ), mCurrentTetrimino( ::model::Tetrimino::Spawn( ) ), mStage( window )
{
	gService()->audio().stopBGM( );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn( ) );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn( ) );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn( ) );
	
	loadResources( );
	mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
}

void ::scene::inPlay::Playing::loadResources( )
{
	uint32_t backgroundColor = 0x29cdb5'fa;
	sf::Vector2f stagePanelPosition( 130.0, 0.0 );
	float stageCellSize = 30.0;
	uint32_t stagePanelColor = 0x3f3f3f'ff;
	float stagePanelOutlineThickness = 11.0;
	uint32_t stagePanelOutlineColor = 0x3f3f3f'7f;
	uint32_t stageCellOutlineColor = 0x000000'7f;
	std::string vfxComboSpritePathNName( "Vfxs/Combo.png" );
	sf::Vector2i vfxComboClipSize( 256, 256 );
	sf::Vector2f nextTetPanelPosition( 525.0, 70.0 );
	float nextTetPanelCellSize = 30.0;
	uint32_t nextTetPanelColor = 0x000000'ff;
	float nextTetPanelOutlineThickness = 5.0;
	uint32_t nextTetPanelOutlineColor = 0x000000'7f;
	uint32_t nextTetPanelCellOutlineColor = 0x000000'7f;
	mAudioList[(int)AudioIndex::TETRIMINO_LOCKED] = "Audio/tetriminoLocked.wav";
	mAudioList[(int)AudioIndex::LINE_CLEARED] = "Audio/lineCleared.wav";

	lua_State* lua = luaL_newstate( );
	const std::string scriptPathNName( "Scripts/Playing.lua" );
	if ( true == luaL_dofile(lua, scriptPathNName.data()) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL,
											 "File Not Found: "+scriptPathNName );
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

		std::string tableName( "PlayerPanel" );
		lua_getglobal( lua, tableName.data( ) );
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
				stagePanelPosition.x = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				stagePanelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				stageCellSize = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				stagePanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				stagePanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				stagePanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "cellOutlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				stageCellOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													tableName, scriptPathNName );
		}
		else
		{
			std::string field( "path" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TSTRING == type )
			{
				vfxComboSpritePathNName = lua_tostring(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
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
			type = lua_type( lua, TOP_IDX );
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
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName, scriptPathNName );
		}
		else
		{
			std::string field( "x" );
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelPosition.x = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelPosition.y = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelCellSize = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
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
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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

			field = "cellOutlineColor";
			lua_pushstring( lua, field.data() );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelCellOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
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
			std::string innerTableName( "tetriminoLocked" );
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
					mAudioList[(int)AudioIndex::TETRIMINO_LOCKED] = lua_tostring(lua, TOP_IDX);
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

			innerTableName = "lineCleared";
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
					mAudioList[(int)AudioIndex::LINE_CLEARED] = lua_tostring(lua, TOP_IDX);
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

	if ( false == mVfxCombo.loadResources( vfxComboSpritePathNName ) )
	{
		gService( )->console( ).printFailure( FailureLevel::FATAL,
												"File Not Found: "+vfxComboSpritePathNName );
#ifdef _DEBUG
		__debugbreak( );
#endif
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
	mCellSize_ = stageCellSize;
}

::scene::inPlay::ID scene::inPlay::Playing::update( std::vector<sf::Event>& eventQueue )
{
	const uint32_t fps = (uint32_t)gService( )->vault( )[HK_FORE_FPS];
	if ( fps < mFrameCount_gameOver )
	{
		return ::scene::inPlay::ID::GAME_OVER;
	}
	else if ( 0u != mFrameCount_gameOver )
	{
		return ::scene::inPlay::ID::AS_IS;
	}

	::scene::inPlay::ID retVal = ::scene::inPlay::ID::AS_IS;
	bool hasCollidedAtThisFrame = false;
	if ( true == mCurrentTetrimino.isFallingDown( ) )
	{
		for ( uint8_t i = 0u; i != FALLING_DOWN_SPEED; ++i )
		{
			hasCollidedAtThisFrame = mCurrentTetrimino.moveDown(mStage.cgrid());
			if ( true == hasCollidedAtThisFrame )
			{
				mCurrentTetrimino.fallDown( false );
				// NOTE: Break the loop and stop stuff in the 1st if-scope immediately.
				goto last;
			}
		}
		return ::scene::inPlay::ID::AS_IS;
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
						mCurrentTetrimino.fallDown( );
						// NOTE: Don't 'return', or it can't come out of the infinite loop.
						///return;
						[[ fallthrough ]];
					case sf::Keyboard::Down:
						hasCollidedAtThisFrame = mCurrentTetrimino.moveDown( mStage.cgrid( ) );
						mFrameCount_fallDown = 0u;
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
	
	if ( static_cast<uint32_t>(fps*mTempo) < mFrameCount_fallDown )
	{
		hasCollidedAtThisFrame = mCurrentTetrimino.moveDown( mStage.cgrid( ) );
		mFrameCount_fallDown = 0u;
	}

	last:
	if ( true == hasCollidedAtThisFrame )
	{
		mCurrentTetrimino.land( mStage.grid() );
		if ( false == gService()->audio().playSFX(mAudioList[(int)AudioIndex::TETRIMINO_LOCKED]) )
		{
			gService()->console().printFailure(FailureLevel::WARNING,
											   "File Not Found: "+mAudioList[(int)AudioIndex::TETRIMINO_LOCKED] );
		}
		reloadTetrimino( );
	}

	// Check if a row or more have to be cleared,
	// NOTE: It's better to check that every several frames than every frame.
	if ( (uint32_t)fps*LINE_CLEAR_CHK_INTERVAL_MS/1000 < mFrameCount_clearingInterval_ )
	{
		const uint8_t numOfLinesCleared = mStage.tryClearRow( );
		mFrameCount_clearingInterval_ = 0u;
		if ( 0 != numOfLinesCleared )
		{
			mNumOfLinesCleared = numOfLinesCleared;
			mTempo -= TEMPO_DIFF_RATIO;
			// Triggering.
			mFrameCount_clearingVfx_ = 1;
			if ( false == gService()->audio().playSFX(mAudioList[(int)AudioIndex::LINE_CLEARED]) )
			{
				gService()->console().printFailure(FailureLevel::WARNING,
												   "File Not Found: "+mAudioList[(int)AudioIndex::LINE_CLEARED] );
			}
		}
		if ( true == mStage.isOver( ) )
		{
			mStage.blackout( );
			const sf::Color GRAY( 0x808080ff );
			mCurrentTetrimino.setColor( GRAY, GRAY );
			// Triggering.
			mFrameCount_gameOver = 1;
		}
	}
	
	return retVal;
}

void ::scene::inPlay::Playing::draw( )
{
	mWindow_.draw( mBackgroundRect_ ); //TODO: Z 버퍼로 컬링해서 부하를 줄여볼까?
	mStage.draw( );
	mCurrentTetrimino.draw( mWindow_ );
	mNextTetriminoPanel.draw( );
	if ( 0 != mFrameCount_clearingVfx_ )
	{
		mVfxCombo.draw( mNumOfLinesCleared );
		++mFrameCount_clearingVfx_;
	}
	
	const uint32_t fps = (uint32_t)gService( )->vault( )[ HK_FORE_FPS ];
	if ( fps <= mFrameCount_clearingVfx_ )
	{
		mFrameCount_clearingVfx_ = 0;
	}
	++mFrameCount_fallDown;
	++mFrameCount_clearingInterval_;
	if ( 0 != mFrameCount_gameOver )
	{
		++mFrameCount_gameOver;
	}
}

void scene::inPlay::Playing::reloadTetrimino()
{
	mCurrentTetrimino = mNextTetriminos.front( );
	mCurrentTetrimino.setOrigin( mStage.position() );
	mCurrentTetrimino.setSize( mCellSize_ );
	mNextTetriminos.pop( );
	mNextTetriminos.emplace( ::model::Tetrimino::Spawn() );
	mNextTetriminoPanel.setTetrimino( mNextTetriminos.front() );
	mFrameCount_fallDown = 0u;
}
