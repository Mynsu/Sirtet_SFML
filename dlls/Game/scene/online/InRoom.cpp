#include "../../pch.h"
#include "InRoom.h"
#include "Online.h"
#include "../../ServiceLocatorMirror.h"
#include "../CommandList.h"
#include "../VaultKeyList.h"
#include <utility>

bool scene::online::InRoom::IsInstantiated = false;

scene::online::InRoom::InRoom( sf::RenderWindow& window, Online& net, const bool asHost )
	: mAsHost( asHost ), mIsReceiving( false ), mHasCanceled( false ), mIsPlaying_( false ),
	mWindow_( window ), mNet( net ),
	mCountdownSpriteClipSize_( 256, 256 ), mCountdownSpritePathNName_( "Images/Countdown.png" ),
	mOtherPlayerSlots{ 0 }
{
	mParticipants.reserve( ROOM_CAPACITY );
	const std::string& nickname = mNet.myNickname( );
	mMyNicknameHashed_ = ::util::hash::Digest( nickname.data(), (uint8_t)nickname.size() );
	mParticipants.emplace( mMyNicknameHashed_, ::ui::PlayView(mWindow_, mNet) );
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
	sf::Vector2i vfxComboClipSize( 256, 256 );
	sf::Vector2f nextTetPanelPos( 520.f, 30.f );
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
				mCountdownSpritePathNName_ = lua_tostring(lua, TOP_IDX);
			}
			// Type Check Exception
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName1+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "clipWidth";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				mCountdownSpriteClipSize_.x = (int)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName1+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "clipHeight";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				mCountdownSpriteClipSize_.y = (int)lua_tointeger(lua, TOP_IDX);
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
				if ( auto it = mParticipants.find(mMyNicknameHashed_);
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
#ifdef _DEBUG
				// Exception
				else
				{
					__debugbreak( );
				}
#endif
			}
			// Type Check Exception
			else
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName2+":"+field0).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field1[ ] = "clipWidth";
			lua_pushstring( lua, field1 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				vfxComboClipSize.x = (int)lua_tointeger(lua, TOP_IDX);
			}
			// Type Check Exception
			else if ( LUA_TNIL != type )
			{
				gService( )->console( ).printScriptError( ExceptionType::TYPE_CHECK,
					(tableName2+":"+field1).data(), scriptPathNName );
			}
			lua_pop( lua, 1 );

			const char field2[ ] = "clipHeight";
			lua_pushstring( lua, field2 );
			lua_gettable( lua, 1 );
			type = lua_type( lua, TOP_IDX );
			// Type check
			if ( LUA_TNUMBER == type )
			{
				vfxComboClipSize.y = (int)lua_tointeger(lua, TOP_IDX);
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
	if ( auto it = mParticipants.find(mMyNicknameHashed_);
		mParticipants.end() != it )
	{
		::ui::PlayView& playView = it->second;
		::model::Stage& stage = playView.stage();
		stage.setPosition( stagePanelPos );
		stage.setSize( stageCellSize );
		stage.setBackgroundColor( sf::Color(stagePanelColor),
								 stagePanelOutlineThickness, sf::Color(stagePanelOutlineColor),
								 sf::Color(stageCellOutlineColor) );
		if ( false == it->second.loadCountdownSprite(mCountdownSpritePathNName_) )
		{
			// File Not Found Exception
			gService( )->console( ).printScriptError( ExceptionType::FILE_NOT_FOUND,
				"Path:CountdownSprite", scriptPathNName );
			std::string defaultPathNName( "Images/Countdown.png" );
			if ( false == it->second.loadCountdownSprite(defaultPathNName) )
			{
				gService()->console().printFailure( FailureLevel::WARNING,
												   "Can't find a countdown sprite image." );
#ifdef _DEBUG
				__debugbreak( );
#endif
			}
		}
		playView.setCountdownSpriteDimension( stagePanelPos, stageCellSize, mCountdownSpriteClipSize_ );
		::model::Tetrimino& tetrimino =	playView.currentTetrimino();
		tetrimino.setOrigin( stagePanelPos );
		tetrimino.setSize( stageCellSize );
		::vfx::Combo& vfxCombo = playView.vfxCombo();
		vfxCombo.setOrigin( stagePanelPos, stageCellSize, vfxComboClipSize );
		::ui::NextTetriminoPanel& nextTetPanel = playView.nextTetriminoPanel();
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
#ifdef _DEBUG
	else
	{
		__debugbreak( );
	}
#endif

	sf::Vector2f otherPlayerSlotPos( 600.f, 400.f );
	mOtherPlayerStageCellSize_ = 8.f;
	const sf::Vector2f size( ::model::stage::GRID_WIDTH*mOtherPlayerStageCellSize_,
							::model::stage::GRID_HEIGHT*mOtherPlayerStageCellSize_ );
	sf::Vector2f diff( 0.f, mOtherPlayerStageCellSize_*::model::stage::GRID_HEIGHT+10.f );
	mOtherPlayerSlotBackgrounds[ 0 ].setFillColor( sf::Color(stagePanelColor) );
	mOtherPlayerSlotBackgrounds[ 0 ].setPosition( otherPlayerSlotPos - diff );
	mOtherPlayerSlotBackgrounds[ 0 ].setSize( size );
	mOtherPlayerSlotBackgrounds[ 0 ].setOutlineThickness( 1.f );
	mOtherPlayerSlotBackgrounds[ 0 ].setOutlineColor( sf::Color(stagePanelOutlineColor) );

	mOtherPlayerSlotBackgrounds[ 1 ].setFillColor( sf::Color(stagePanelColor) );
	diff = sf::Vector2f( mOtherPlayerStageCellSize_*::model::stage::GRID_WIDTH+10.f, 0.f );
	mOtherPlayerSlotBackgrounds[ 1 ].setPosition( otherPlayerSlotPos - diff );
	mOtherPlayerSlotBackgrounds[ 1 ].setSize( size );
	mOtherPlayerSlotBackgrounds[ 1 ].setOutlineThickness( 1.f );
	mOtherPlayerSlotBackgrounds[ 1 ].setOutlineColor( sf::Color(stagePanelOutlineColor) );

	mOtherPlayerSlotBackgrounds[ 2 ].setFillColor( sf::Color(stagePanelColor) );
	mOtherPlayerSlotBackgrounds[ 2 ].setPosition( otherPlayerSlotPos );
	mOtherPlayerSlotBackgrounds[ 2 ].setSize( size );
	mOtherPlayerSlotBackgrounds[ 2 ].setOutlineThickness( 1.f );
	mOtherPlayerSlotBackgrounds[ 2 ].setOutlineColor( sf::Color(stagePanelOutlineColor) );

	::model::Tetrimino::LoadResources( );
}

::scene::online::ID scene::online::InRoom::update( std::list<sf::Event>& eventQueue )
{
	::scene::online::ID nextSceneID = ::scene::online::ID::AS_IS;
	if ( true == mNet.hasReceived() )
	{
		mIsReceiving = false;
		const HashedKey myNicknameHashed = mNet.myNicknameHashed();
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
			std::unordered_map< HashedKey, std::string > users;
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
							nicknameHashed = 0;
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
				mParticipants.emplace( pair.first, ::ui::PlayView(mWindow_, mNet, false) );
				uint8_t slotIdx = 0;
				while ( ROOM_CAPACITY-1 != slotIdx )
				{
					if ( 0 == mOtherPlayerSlots[slotIdx] )
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
				::model::Stage& stage =	mParticipants[pair.first].stage();
				sf::RectangleShape& slotBg = mOtherPlayerSlotBackgrounds[slotIdx];
				// TODO: 닉네임 보여주기
				const sf::Vector2f otherPStagePos( slotBg.getPosition() );
				stage.setPosition( otherPStagePos );
				stage.setSize( mOtherPlayerStageCellSize_ );
				const sf::Color otherPStageColor = slotBg.getFillColor();
				const float otherPStageOutlineThickness = 5.f;
				const sf::Color otherPStageOutlineColor = slotBg.getOutlineColor();
				const sf::Color otherPStageCellOutlineColor( 0x000000'7f );
				stage.setBackgroundColor( otherPStageColor, otherPStageOutlineThickness, otherPStageOutlineColor,
										 otherPStageCellOutlineColor );
				::model::Tetrimino& curTet = mParticipants[pair.first].currentTetrimino();
				curTet.setOrigin( otherPStagePos );
				curTet.setSize( mOtherPlayerStageCellSize_ );
				if ( false == mParticipants[pair.first].loadCountdownSprite(mCountdownSpritePathNName_) )
				{
					gService()->console().printFailure( FailureLevel::WARNING,
														"Can't find a countdown sprite." );
#ifdef _DEBUG
					__debugbreak( );
#endif
				}
				mParticipants[pair.first].setCountdownSpriteDimension( otherPStagePos,
																	mOtherPlayerStageCellSize_,
																	mCountdownSpriteClipSize_ );
			}
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
					it.second.start( );
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
					if ( nicknameHashed == myNicknameHashed )
					{
						curPos += sizeof(HashedKey) + sizeof(uint8_t);
						continue;
					}
					curPos += sizeof(HashedKey);
					const ::model::tetrimino::Type newType = (::model::tetrimino::Type)ptr[curPos++];
					if ( const auto it = mParticipants.find(nicknameHashed);
						mParticipants.end() != it )
					{
						it->second.setNewCurrentTetrimino( newType );
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
						::model::Tetrimino& tet = it->second.currentTetrimino();
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
						::model::Stage& stage =	it->second.stage();
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
						it->second.setNumOfLinesCleared( numOfLinesCleared );
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
						it->second.gameOver();
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
					pair.second.gameOver( );
				}
				mIsPlaying_ = false;
			}
		}
	}
	
	for ( auto& pair : mParticipants )
	{
		pair.second.update( eventQueue );
	}

	if ( false == mIsReceiving )
	{
		mNet.receive( );
		mIsReceiving = true;
	}

	if ( true == mHasCanceled )
	{
		nextSceneID = ::scene::online::ID::IN_LOBBY;
	}

	return nextSceneID;
}

void scene::online::InRoom::draw( )
{
	mWindow_.draw( mBackgroundRect );
	uint8_t filled = 0;
	for ( auto& it : mParticipants )
	{
		it.second.draw( );
		++filled;
	}
	for ( size_t i = filled-1; ROOM_CAPACITY-1 != i; ++i )
	{
		mWindow_.draw( mOtherPlayerSlotBackgrounds[i] );
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

