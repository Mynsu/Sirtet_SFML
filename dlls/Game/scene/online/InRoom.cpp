#include "../../pch.h"
#include "InRoom.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "../VaultKeyList.h"
#include <utility>

const uint8_t ROOM_CAPACITY = 4;

bool scene::online::InRoom::IsInstantiated = false;

scene::online::InRoom::InRoom( sf::RenderWindow& window, Online& net, const bool asHost )
	: mAsHost( asHost ), mIsReceiving( false ), mHasCanceled( false ),
	mWindow_( window ), mNet( net )
{
	const std::string& nickname = mNet.myNickname( );
	mParticipants.reserve( ROOM_CAPACITY );
	mMyNicknameHash_ = ::util::hash::Digest( nickname.data(), (uint8_t)nickname.size() );
	mParticipants.emplace( mMyNicknameHash_, ::ui::PlayView(mWindow_, mNet) );
	loadResources( );
#ifdef _DEBUG
	gService( )->console( ).print( "Now in a room.", sf::Color::Green );
#endif
#ifdef _DEV
	gService( )->console( ).addCommand( CMD_LEAVE_ROOM, std::bind( &scene::online::InRoom::leaveRoom,
																 this, std::placeholders::_1 ) );
	if ( true == asHost )
	{
		gService( )->console( ).addCommand( CMD_START_GAME, std::bind( &scene::online::InRoom::startGame,
																	 this, std::placeholders::_1 ) );
	}
#endif
	IsInstantiated = true;
}

scene::online::InRoom::~InRoom( )
{
#ifdef _DEV
	if ( nullptr != gService() )
	{
		gService( )->console( ).removeCommand( CMD_LEAVE_ROOM );
		if ( true == mAsHost )
		{
			gService( )->console( ).removeCommand( CMD_START_GAME );
		}
	}
#endif
	IsInstantiated = false;
}

void scene::online::InRoom::loadResources( )
{
	uint32_t backgroundColor = 0x8ae5ff'ff;
	sf::Vector2f stagePanelPos( 100.0, 0.0 );
	float stageCellSize = 30.0;
	uint32_t stagePanelColor = 0x3f3f3f'ff;
	float stagePanelOutlineThickness = 11.0;
	uint32_t stagePanelOutlineColor = 0x3f3f3f'7f;
	uint32_t stageCellOutlineColor = 0x000000'7f;
	sf::Vector2i countdownSpriteSize( 256, 256 );
	sf::Vector2i vfxComboSize( 256, 256 );
	sf::Vector2f nextTetPanelPos( 420.f, 30.f );
	float nextTetPanelCellSize = 30.0;
	uint32_t nextTetPanelColor = 0x000000'ff;
	float nextTetPanelOutlineThickness = 5.0;
	uint32_t nextTetPanelOutlineColor = 0x000000'7f;
	uint32_t nextTetPanelCellOutlineColor = 0x000000'7f;
	bool isDefault = true;

	lua_State* lua = luaL_newstate( );
	const char scriptPathNName[] = "Scripts/InRoom.lua";
	if ( true == luaL_dofile(lua, scriptPathNName) )
	{
		// File Not Found Exception
		gService( )->console( ).printFailure( FailureLevel::FATAL, std::string("File Not Found: ")+scriptPathNName );
		lua_close( lua );
	}
	else
	{
		luaL_openlibs( lua );
		const int TOP_IDX = -1;

		const std::string valName0( "BackgroundColor" );
		lua_getglobal( lua, valName0.data() );
		if ( false == lua_isinteger(lua, TOP_IDX) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 valName0.data( ), scriptPathNName );
		}
		else
		{
			backgroundColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
		}
		lua_pop( lua, 1 );

		const std::string tableName0( "PlayerPanel" );
		lua_getglobal( lua, tableName0.data( ) );
		if ( false == lua_istable(lua, TOP_IDX) )
		{
			// Type Check Exception
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName0.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "x";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				stagePanelPos.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "y";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				stagePanelPos.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "cellSize";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				stageCellSize = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field2).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field3[ ] = "color";
			lua_pushstring( lua, field3 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				stagePanelColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field3).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field4[ ] = "outlineThickness";
			lua_pushstring( lua, field4 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				stagePanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field4).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field5[ ] = "outlineColor";
			lua_pushstring( lua, field5 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				stagePanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field5).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field6[ ] = "cellOutlineColor";
			lua_pushstring( lua, field6 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				stageCellOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName0+":"+field6).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		const std::string tableName1( "CountdownSprite" );
		lua_getglobal( lua, tableName1.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName1.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "path";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TSTRING == type )
			{
				if ( auto it = mParticipants.find(mMyNicknameHash_);
					mParticipants.end() != it )
				{
					std::string filePathNName( lua_tostring(lua, TOP_IDX) );
					if ( false == it->second.loadTexture(filePathNName) )
					{
						// File Not Found Exception
						gService( )->console( ).printScriptError( ExceptionType::FILE_NOT_FOUND,
							(tableName1+":"+field0).data(), scriptPathNName );
					}
					else
					{
						isDefault = false;
					}
				}
				// Exception
				else
				{
				}
			}
			// Type Check Exception
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName1+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				countdownSpriteSize.x = (int)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName1+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "height";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				countdownSpriteSize.y = (int)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName1+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		const std::string tableName2( "VfxCombo" );
		lua_getglobal( lua, tableName2.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName2.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "path";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TSTRING == type )
			{
				if ( auto it = mParticipants.find(mMyNicknameHash_);
					mParticipants.end() != it )
				{
					std::string filePathNName( lua_tostring(lua, TOP_IDX) );
					if ( false == it->second.vfxCombo().loadResources(filePathNName) )
					{
						// File Not Found Exception
						gService( )->console( ).printScriptError( ExceptionType::FILE_NOT_FOUND,
							(tableName2+":"+field0).data(), scriptPathNName );
					}
					else
					{
						isDefault = false;
					}
				}
				// Exception
				else
				{
				}
			}
			// Type Check Exception
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName2+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "width";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				vfxComboSize.x = (int)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName2+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "height";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				vfxComboSize.y = (int)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName2+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}

		const std::string tableName3( "NextTetriminoPanel" );
		lua_getglobal( lua, tableName3.data( ) );
		// Type Check Exception
		if ( false == lua_istable( lua, TOP_IDX ) )
		{
			gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
													 tableName3.data(), scriptPathNName );
		}
		else
		{
			const char field0[ ] = "x";
			lua_pushstring( lua, field0 );
			lua_gettable( lua, 1 );
			int type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelPos.x = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName3+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "y";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelPos.y = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName3+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "cellSize";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelCellSize = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName3+":"+field2).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field3[ ] = "color";
			lua_pushstring( lua, field3 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelColor = (uint32_t)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName3+":"+field3).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field4[ ] = "outlineThickness";
			lua_pushstring( lua, field4 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelOutlineThickness = (float)lua_tonumber(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName3+":"+field4).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field5[ ] = "outlineColor";
			lua_pushstring( lua, field5 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName3+":"+field5).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field6[ ] = "cellOutlineColor";
			lua_pushstring( lua, field6 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				nextTetPanelCellOutlineColor = (uint32_t)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName3+":"+field6).data(), scriptPathNName );
			}
			lua_pop( lua, 2 );
		}
		lua_close( lua );
	}

	mBackgroundRect.setSize( sf::Vector2f(mWindow_.getSize()) );
	mBackgroundRect.setFillColor( sf::Color(backgroundColor) );
	if ( auto it = mParticipants.find(mMyNicknameHash_);
		mParticipants.end() != it )
	{
		::model::Stage& stage = it->second.stage();
		stage.setPosition( stagePanelPos );
		stage.setSize( stageCellSize );
		stage.setBackgroundColor( sf::Color(stagePanelColor),
								 stagePanelOutlineThickness, sf::Color(stagePanelOutlineColor),
								 sf::Color(stageCellOutlineColor) );
		it->second.setCountdownSpriteDimension( stagePanelPos, stageCellSize, countdownSpriteSize );
		::model::Tetrimino& tetrimino =	it->second.tetrimino();
		tetrimino.setOrigin( stagePanelPos );
		tetrimino.setSize( stageCellSize );
		::vfx::Combo& vfxCombo = it->second.vfxCombo();
		vfxCombo.setOrigin( stagePanelPos, stageCellSize, vfxComboSize );
		::ui::NextTetriminoPanel& nextTetPanel = it->second.nextTetriminoPanel();
		nextTetPanel.setDimension( nextTetPanelPos, nextTetPanelCellSize );
		nextTetPanel.setBackgroundColor( sf::Color(nextTetPanelColor),
										nextTetPanelOutlineThickness, sf::Color(nextTetPanelOutlineColor),
										sf::Color(nextTetPanelCellOutlineColor) );
		if ( true == isDefault )
		{
			const char defaultFilePathNName[] = "Vfxs/Combo.png";
			if ( false == vfxCombo.loadResources( defaultFilePathNName ) )
			{
				// Exception: When there's not even the default file,
				gService( )->console( ).printFailure( FailureLevel::FATAL,
													 std::string("File Not Found: ")+defaultFilePathNName );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}
	}
	// Exception
	else
	{

	}

	::model::Tetrimino::LoadResources( );
}

::scene::online::ID scene::online::InRoom::update( std::list<sf::Event>& eventQueue )
{
	if ( false == mIsReceiving )
	{
		mNet.receive( );
		mIsReceiving = true;
	}
	
	bool hasToRespond = false;
	if ( true == mNet.hasReceived() )
	{
		mIsReceiving = false;
		if ( std::optional<std::string> response(mNet.getByTag(TAGGED_REQ_GET_READY,
																Online::Option::RETURN_TAG_ATTACHED));
			 std::nullopt != response )
		{
			for ( auto& it : mParticipants )
			{
				it.second.start( );
			}
		}
		hasToRespond = true;
	}

	for ( auto& it : mParticipants )
	{
		hasToRespond = it.second.update( eventQueue );
	}

	if ( true == hasToRespond )
	{
		mNet.sendZeroByte( );
	}

	::scene::online::ID retVal = ::scene::online::ID::AS_IS;
	if ( true == mHasCanceled )
	{
		retVal = ::scene::online::ID::IN_LOBBY;
	}

	return retVal;
}

void scene::online::InRoom::draw( )
{
	mWindow_.draw( mBackgroundRect );
	for ( auto& it : mParticipants )
	{
		it.second.draw( );
	}
}

void scene::online::InRoom::startGame( const std::string_view& arg )
{
	std::string request( TAGGED_REQ_START_GAME );
	mNet.send( request.data(), (int)request.size() );
}

void scene::online::InRoom::leaveRoom( const std::string_view& arg )
{
	std::string request( TAGGED_REQ_LEAVE_ROOM );
	mNet.send( request.data(), (int)request.size() );
	mHasCanceled = true;
}

